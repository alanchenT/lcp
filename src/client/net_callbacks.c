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

bool handle_set_display_name(PeerList* peers, void* packet) {
    PacketSetDisplayName* data = packet;

    set_peer_display_name(peers, data->client_id, data->display_name);

    return true;
}

bool handle_client_connect(PeerList* peers, void* packet) {
    PacketClientConnect* data = packet;

    add_peer(peers, data->client_id);

    return true;
}

bool handle_client_disconnect(PeerList* peers, void* packet) {
    PacketClientDisconnect* data = packet;

    remove_peer(peers, data->client_id);

    return true;
}

bool handle_client_list(PeerList* peers, void* packet) {
    PacketClientList* data = packet;

    for (size_t i = 0; i < data->list_len; ++i) {
        ReplicatedClientContext* ctx = &data->list[i];

        add_peer(peers, ctx->id);
        set_peer_display_name(peers, ctx->id, ctx->display_name);
    }

    print_peer_list(peers);

    return true;
}