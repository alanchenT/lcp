#include "client/client.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "net_shared.h"
#include "payload_queue.h"

static void* client_send_thread(void* arg) {
    Client* client = arg;

    while (client->is_connected) {
        Payload* payload = pop_payload_queue(client->send_queue);

        if (payload == NULL) {
            break;
        }

        bool send_status = net_send(client->socket_fd, payload->data, payload->len);
        if (!send_status) {
            client->is_connected = false;
            free_payload(payload);
            break;
        }

        free_payload(payload);
    }

    return NULL;
}

static void* client_recv_thread(void* arg) {
    Client* client = arg;

    uint8_t recv_buffer[MAX_NET_BUFFER_SIZE];

    while (client->is_connected) {
        size_t payload_len = net_recv(client->socket_fd, recv_buffer, sizeof(recv_buffer));
        if (payload_len == 0) { // Disconnected / error
            client->is_connected = false;
            break;
        }

        printf("received (%ld): %s\n", payload_len, recv_buffer);
        push_payload_queue(client->recv_queue, recv_buffer, payload_len);
    }

    return NULL;
}

Client* alloc_client(void) {
    Client* client = malloc(sizeof(Client));
    if (client == NULL) {
        return NULL;
    }

    client->socket_fd = -1;
    client->is_connected = false;
    client->send_queue = alloc_payload_queue();
    client->recv_queue = alloc_payload_queue();

    return client;
}

static struct addrinfo* find_valid_addrinfo(struct addrinfo* first_addr, int* socket_fd) {
    char ip_string[INET6_ADDRSTRLEN];

    struct addrinfo* current = NULL;

    for (current = first_addr; current != NULL; current = current->ai_next) {
        *socket_fd = socket(current->ai_family, current->ai_socktype, current->ai_protocol);
        if (*socket_fd == -1) {
            perror("[find_valid_addrinfo] socket error");
            continue;
        }

        const char* ntop_status =
            inet_ntop(current->ai_family, extract_sockaddr(current->ai_addr), ip_string, sizeof(ip_string));
        if (ntop_status == NULL) {
            perror("[find_valid_addrinfo] inet_ntop error");
            continue;
        }
        printf("client: connecting to %s\n", ip_string);

        if (connect(*socket_fd, current->ai_addr, current->ai_addrlen) == -1) {
            perror("[find_valid_addrinfo] connect error");
            close(*socket_fd);
            continue;
        }

        break;
    }

    return current;
}

static int open_client_socket(void) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* first_addr = NULL;

    // TODO: Different server ports?
    int status = getaddrinfo(NULL, SERVER_PORT, &hints, &first_addr);
    if (status == -1) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return -1;
    }

    int socket_fd = -1;
    struct addrinfo* server_info = find_valid_addrinfo(first_addr, &socket_fd);

    if (server_info == NULL) {
        fprintf(stderr, "client: failed to connect to a socket\n");
        return -1;
    }

    freeaddrinfo(first_addr);
    first_addr = NULL;
    server_info = NULL;

    return socket_fd;
}

bool client_connect(Client* client) {
    client->socket_fd = open_client_socket();
    if (client->socket_fd == -1) {
        fprintf(stderr, "client: failed to open socket\n");
        return false;
    }

    int pthread_status = 0;

    pthread_status = pthread_create(&client->recv_thread, NULL, client_recv_thread, client);
    if (pthread_status != 0) {
        fprintf(stderr, "client: pthread_create failed (recv)\n");
        return false;
    }

    pthread_status = pthread_create(&client->recv_thread, NULL, client_send_thread, client);
    if (pthread_status != 0) {
        fprintf(stderr, "client: pthread_create failed (send)\n");
        return false;
    }

    client->is_connected = true;

    return true;
}

bool client_disconnect(Client* client) {
    if (!client->is_connected) {
        return false;
    }

    client->is_connected = false;

    shutdown_payload_queue(client->send_queue);
    shutdown_payload_queue(client->recv_queue);

    pthread_join(client->send_thread, NULL);
    pthread_join(client->recv_thread, NULL);

    close(client->socket_fd);

    return true;
}

void client_send(Client* client, Payload* payload) {
    move_into_payload_queue(client->send_queue, payload);
}