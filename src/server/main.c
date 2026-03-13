/*
    main.c (server)
*/

#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common.h"
#include "server/client_list.h"
#include "server/net.h"

int main() {
    int listener_socket_fd = open_server_socket();
    if (listener_socket_fd == -1) {
        fprintf(stderr, "server: failed to open listener socket\n");
        exit(1);
    }

    ClientList* client_list = init_client_list();
    if (client_list == nullptr) {
        exit(1);
    }

    printf("server: waiting for connections...\n");

    struct sockaddr_storage connection_addr;
    socklen_t connection_addr_len = sizeof(connection_addr);

    while (1) {
        int client_fd = accept(listener_socket_fd, (struct sockaddr*)&connection_addr, &connection_addr_len);
        if (client_fd == -1) {
            perror("accept error");
            continue;
        }

        bool success = add_client(client_list, client_fd);
        if (!success) {
            printf("server: client failed to connect\n");
            continue;
        }

        printf("server: a client connected\n");
    }

    close(listener_socket_fd);

    return 0;
}