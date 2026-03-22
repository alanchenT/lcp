#include "server/server.h"

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "server/client_list.h"
#include "server/net_callbacks.h"

Server* alloc_server(void) {
    Server* new_server = malloc(sizeof(Server));
    if (new_server == NULL) {
        return NULL;
    }

    new_server->client_list = alloc_client_list();
    if (new_server->client_list == NULL) {
        perror("[alloc_server]: unable to malloc ClientList");
        free(new_server);
        return NULL;
    }

    new_server->listener_socket_fd = -1;
    new_server->is_running = false;
    pthread_mutex_init(&new_server->is_running_mutex, NULL);

    return new_server;
}

void free_server(Server* server) {
    if (server_is_running(server)) {
        server_stop(server);
    }

    pthread_mutex_destroy(&server->is_running_mutex);
    free_client_list(server->client_list);
    free(server);
}

static struct addrinfo* find_valid_addrinfo(struct addrinfo* first_addr, int* socket_fd) {
    const int yes = 1; // For setsockopt
    struct addrinfo* current = NULL;

    // Iterate through the linked list until we find a working socket address
    for (current = first_addr; current != NULL; current = current->ai_next) {
        *socket_fd = socket(current->ai_family, current->ai_socktype, current->ai_protocol);
        if (*socket_fd == -1) {
            perror("[open_listener_socket] socket error");
            continue;
        }

        if (setsockopt(*socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            perror("[open_listener_socket] setsockopt error");
            continue;
        }

        if (bind(*socket_fd, current->ai_addr, current->ai_addrlen) == -1) {
            close(*socket_fd);
            perror("[open_listener_socket] bind error");
            continue;
        }

        break;
    }

    return current;
}

static int open_server_socket(void) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* first_addr = NULL;

    int status = getaddrinfo(NULL, SERVER_PORT, &hints, &first_addr);
    if (status == -1) {
        fprintf(stderr, "[open_listener_socket] getaddrinfo error: %s\n", gai_strerror(status));
        return -1;
    }

    int listener_socket_fd = -1;
    struct addrinfo* server_info = find_valid_addrinfo(first_addr, &listener_socket_fd);

    if (server_info == NULL) {
        fprintf(stderr, "[open_listener_socket] failed to bind to a socket\n");
        return -1;
    }

    // Clean up all addrinfos
    freeaddrinfo(first_addr);
    first_addr = NULL;
    server_info = NULL;

    if (listen(listener_socket_fd, SERVER_LISTENER_BACKLOG) == -1) {
        perror("[open_listener_socket] listen error");
        close(listener_socket_fd);
        return -1;
    }

    return listener_socket_fd;
}

void* accept_loop(void* arg) {
    Server* server = arg;

    printf("server: waiting for connections...\n");

    struct sockaddr_storage connection_addr;
    socklen_t connection_addr_len = sizeof(connection_addr);

    while (server_is_running(server)) {
        int client_fd = accept(server->listener_socket_fd, (struct sockaddr*)&connection_addr, &connection_addr_len);
        if (client_fd == -1) {
            if (server_is_running(server)) {
                perror("accept error");
            }

            continue;
        }

        if (!server_is_running(server)) {
            close(client_fd);
            break;
        }

        ClientContext* ctx = add_client(server->client_list, client_fd);
        if (ctx == NULL) {
            printf("server: client failed to connect\n");
            continue;
        }

        handle_client_connect(ctx);

        printf("server: a client connected\n");
    }

    return NULL;
}

bool server_start(Server* server) {
    int listener_socket_fd = open_server_socket();
    if (listener_socket_fd == -1) {
        return false;
    }

    server->listener_socket_fd = listener_socket_fd;

    pthread_mutex_lock(&server->is_running_mutex);
    server->is_running = true;
    pthread_mutex_unlock(&server->is_running_mutex);

    pthread_create(&server->accept_thread, NULL, accept_loop, server);

    return true;
}

void server_stop(Server* server) {
    pthread_mutex_lock(&server->is_running_mutex);
    server->is_running = false;
    pthread_mutex_unlock(&server->is_running_mutex);

    if (server->listener_socket_fd != -1) {
        close(server->listener_socket_fd);
        server->listener_socket_fd = -1;
    }

    pthread_join(server->accept_thread, NULL);

    remove_all_clients(server->client_list);
}

bool server_is_running(Server* server) {
    pthread_mutex_lock(&server->is_running_mutex);
    bool is_running = server->is_running;
    pthread_mutex_unlock(&server->is_running_mutex);

    return is_running;
}