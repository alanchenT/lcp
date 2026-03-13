#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "client/net.h"
#include "common.h"
#include "debug.h"

int main() {
    int socket_fd = open_client_socket();
    if (socket_fd == -1) {
        fprintf(stderr, "client: failed to open socket\n");
        exit(1);
    }

    display_peer_info(socket_fd);

    char message[MAX_NET_BUFFER_SIZE];
    while (1) {
        printf("message: ");

        if (fgets(message, sizeof(message), stdin) == nullptr) {
            break;
        }

        message[strcspn(message, "\n")] = '\0';

        net_send(socket_fd, message, strlen(message) + 1);
    }

    close(socket_fd);
    return 0;
}