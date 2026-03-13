#include "server/net.h"

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common.h"

static struct addrinfo* find_valid_addrinfo(struct addrinfo* first_addr, int* socket_fd) {
    const int yes = 1; // For setsockopt
    struct addrinfo* current = nullptr;

    // Iterate through the linked list until we find a working socket address
    for (current = first_addr; current != nullptr; current = current->ai_next) {
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

int open_server_socket(void) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* first_addr = nullptr;

    int status = getaddrinfo(nullptr, SERVER_PORT, &hints, &first_addr);
    if (status == -1) {
        fprintf(stderr, "[open_listener_socket] getaddrinfo error: %s\n", gai_strerror(status));
        return -1;
    }

    int listener_socket_fd = -1;
    struct addrinfo* server_info = find_valid_addrinfo(first_addr, &listener_socket_fd);

    if (server_info == nullptr) {
        fprintf(stderr, "[open_listener_socket] failed to bind to a socket\n");
        return -1;
    }

    // Clean up all addrinfos
    freeaddrinfo(first_addr);
    first_addr = nullptr;
    server_info = nullptr;

    if (listen(listener_socket_fd, SERVER_LISTENER_BACKLOG) == -1) {
        perror("[open_listener_socket] listen error");
        close(listener_socket_fd);
        return -1;
    }

    return listener_socket_fd;
}
