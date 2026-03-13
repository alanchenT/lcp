#include "client/net.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common.h"

static struct addrinfo* find_valid_addrinfo(struct addrinfo* first_addr, int* socket_fd) {
    char ip_string[INET6_ADDRSTRLEN];

    struct addrinfo* current = nullptr;

    for (current = first_addr; current != nullptr; current = current->ai_next) {
        *socket_fd = socket(current->ai_family, current->ai_socktype, current->ai_protocol);
        if (*socket_fd == -1) {
            perror("[find_valid_addrinfo] socket error");
            continue;
        }

        const char* ntop_status =
            inet_ntop(current->ai_family, extract_sockaddr(current->ai_addr), ip_string, sizeof(ip_string));
        if (ntop_status == nullptr) {
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

int open_client_socket(void) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* first_addr = nullptr;
    int status = getaddrinfo(nullptr, SERVER_PORT, &hints, &first_addr);
    if (status == -1) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return -1;
    }

    int socket_fd = -1;
    struct addrinfo* server_info = find_valid_addrinfo(first_addr, &socket_fd);

    if (server_info == nullptr) {
        fprintf(stderr, "client: failed to connect to a socket\n");
        return -1;
    }

    freeaddrinfo(first_addr);
    first_addr = nullptr;
    server_info = nullptr;

    return socket_fd;
}