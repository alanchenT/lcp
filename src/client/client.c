#include "client/client.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "client/app.h"
#include "client/peer_list.h"
#include "net_shared.h"
#include "packet_queue.h"
#include "packets.h"

#define INVALID_CLIENT_ID -1

static void* client_send_thread(void* arg) {
    Client* client = arg;

    while (client->is_connected) {
        Packet* packet = pop_packet_queue(client->send_queue);

        if (packet == NULL) {
            break;
        }

        bool send_status = net_send(client->socket_fd, packet->internal.payload, packet->internal.payload_len);
        if (!send_status) {
            client->is_connected = false;
            free_packet(packet);
            break;
        }

        free_packet(packet);
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

        if (!recv_packet_queue(client->recv_queue, recv_buffer, payload_len)) {
            break;
        }
    }

    return NULL;
}

Client* alloc_client(void) {
    Client* client = malloc(sizeof(Client));
    if (client == NULL) {
        return NULL;
    }

    client->id = INVALID_CLIENT_ID;
    client->socket_fd = -1;
    client->is_connected = false;
    client->send_queue = alloc_packet_queue();
    client->recv_queue = alloc_packet_queue();
    client->peers = alloc_peer_list();

    memset(client->display_name, 0, sizeof(client->display_name));

    return client;
}

void free_client(Client* client) {
    client_disconnect(client);

    free_packet_queue(client->send_queue);
    free_packet_queue(client->recv_queue);

    free_peer_list(client->peers);
    free(client);
}

bool client_completed_handshake(Client* client) {
    return client->id > INVALID_CLIENT_ID;
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

static int open_client_socket(const char* port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* first_addr = NULL;

    // TODO: Different server ports?
    int status = getaddrinfo(NULL, port, &hints, &first_addr);
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

bool client_connect(Client* client, const char* port) {
    client->socket_fd = open_client_socket(port);
    if (client->socket_fd == -1) {
        fprintf(stderr, "client: failed to open socket\n");
        return false;
    }

    client->is_connected = true;

    int pthread_status = 0;

    pthread_status = pthread_create(&client->recv_thread, NULL, client_recv_thread, client);
    if (pthread_status != 0) {
        fprintf(stderr, "client: pthread_create failed (recv)\n");
        return false;
    }

    pthread_status = pthread_create(&client->send_thread, NULL, client_send_thread, client);
    if (pthread_status != 0) {
        fprintf(stderr, "client: pthread_create failed (send)\n");
        return false;
    }

    return true;
}

bool client_disconnect(Client* client) {
    if (!client->is_connected) {
        return false;
    }

    client->is_connected = false;
    client->id = INVALID_CLIENT_ID;

    shutdown_packet_queue(client->send_queue);
    shutdown_packet_queue(client->recv_queue);

    pthread_join(client->send_thread, NULL);
    pthread_join(client->recv_thread, NULL);

    close(client->socket_fd);

    return true;
}

// TODO: Error handling
void client_send(Client* client, void* packet) {
    write_packet_queue(client->send_queue, packet);
}