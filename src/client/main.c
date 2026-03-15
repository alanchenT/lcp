#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "client/client.h"
#include "common.h"
#include "debug.h"
#include "packets.h"
#include "payload_queue.h"

void message_loop(Client* client) {}

int main() {
    Client* client = alloc_client();
    if (client == NULL) {
        fprintf(stderr, "client: failed to create client\n");
        exit(1);
    }

    if (!client_connect(client)) {
        fprintf(stderr, "client: failed to connect to server\n");
        exit(1);
    }

    display_peer_info(client->socket_fd);

    INIT_PACKET(PacketChatMessage, chat_message)
    chat_message.client_id = client->socket_fd;

    while (client->is_connected) {
        printf("message: ");

        if (fgets(chat_message.message, sizeof(chat_message.message), stdin) == NULL) {
            break;
        }

        chat_message.message[strcspn(chat_message.message, "\n")] = '\0';

        Payload* payload = alloc_payload(sizeof(chat_message));
        payload->len = write_packet(&chat_message, payload->data, payload->len);

        if (payload->len == 0) {
            free_payload(payload);
            continue;
        }
        client_send(client, payload);
    }

    client_disconnect(client);

    return 0;
}