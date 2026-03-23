#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "client/app.h"
#include "client/client.h"
#include "client/gui.h"
#include "client/net_callbacks.h"
#include "debug.h"
#include "packet_queue.h"
#include "packets.h"

void update(Client* client) {
    while (!is_packet_queue_empty(client->recv_queue)) {
        void* packet = poll_packet_queue(client->recv_queue);
        if (packet == NULL) {
            return;
        }

        switch (get_packet_id(packet)) {
            case PACKET_ID(PacketClientHandshake): {
                PacketClientHandshake* handshake = packet;
                client->id = handshake->client_id;
                break;
            }
            case PACKET_ID(PacketChatMessage): {
                handle_chat_message(client, packet);
                break;
            }
            case PACKET_ID(PacketSetDisplayName): {
                handle_set_display_name(client, packet);
                break;
            }
            case PACKET_ID(PacketClientConnect): {
                handle_client_connect(client, packet);
                break;
            }
            case PACKET_ID(PacketClientDisconnect): {
                handle_client_disconnect(client, packet);
                break;
            }
            case PACKET_ID(PacketClientList): {
                handle_client_list(client, packet);
                break;
            }

            default: {
                break;
            }
        }

        free_packet(packet);
    }
}

int main() {
    setenv("GSK_RENDERER", "cairo", 1);

    App* app = alloc_app();
    if (app == NULL) {
        fprintf(stderr, "client: failed to alloc app\n");
        exit(1);
    }

    if (!app_connect(app, SERVER_PORT)) {
        fprintf(stderr, "client: failed to connect to server\n");
        exit(1);
    }

    app_start(app);

    Client* client = alloc_client();
    if (client == NULL) {
        fprintf(stderr, "client: failed to create client\n");
        exit(1);
    }

    if (!client_connect(client, SERVER_PORT)) {
        fprintf(stderr, "client: failed to connect to server\n");
        exit(1);
    }

    while (client->is_connected) {
        update(client);

        ALLOC_PACKET(PacketChatMessage, chat_message);
        chat_message->client_id = client->id;

        printf("message: ");
        if (fgets(chat_message->message, sizeof(chat_message->message), stdin) == NULL) {
            free_packet(chat_message);
            break;
        }

        chat_message->message[strcspn(chat_message->message, "\n")] = '\0';

        client_send(client, chat_message);
    }

    client_disconnect(client);

    return 0;
}