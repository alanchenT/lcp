#include "client/net_callbacks.h"

#include <stdio.h>

#include "client/client.h"
#include "client/peer_list.h"
#include "packet_queue.h"
#include "packets.h"

bool handle_chat_message(Client* client, void* packet) {
    PacketChatMessage* data = packet;

    printf("%d said: %s\n", data->client_id, data->message);

    return true;
}

bool handle_set_display_name(Client* client, void* packet) {
    PacketSetDisplayName* data = packet;

    set_peer_display_name(client->peers, data->client_id, data->display_name);

    return true;
}

bool handle_client_connect(Client* client, void* packet) {
    PacketClientConnect* data = packet;

    add_peer(client->peers, data->client_id);

    return true;
}

bool handle_client_disconnect(Client* client, void* packet) {
    PacketClientDisconnect* data = packet;

    remove_peer(client->peers, data->client_id);

    return true;
}

bool handle_client_list(Client* client, void* packet) {
    PacketClientList* data = packet;

    for (size_t i = 0; i < data->list_len; ++i) {
        ReplicatedClientContext* ctx = &data->list[i];

        add_peer(client->peers, ctx->id);
        set_peer_display_name(client->peers, ctx->id, ctx->display_name);
    }

    print_peer_list(client->peers);

    return true;
}