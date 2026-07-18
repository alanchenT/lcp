#include "client/net_callbacks.h"

#include <stdio.h>

#include "client/app.h"
#include "client/gui.h"
#include "client/peer_list.h"
#include "packet_queue.h"
#include "packets.h"

bool handle_chat_message(App* app, void* packet) {
    PacketChatMessage* data = packet;

    printf("%d said: %s\n", data->client_id, data->message);

    append_chat_message(app->gui, get_peer_display_name(app->peers, data->client_id), data->message);

    return true;
}

bool handle_set_display_name(App* app, void* packet) {
    PacketSetDisplayName* data = packet;

    set_peer_display_name(app->peers, data->client_id, data->display_name);

    return true;
}

bool handle_client_connect(App* app, void* packet) {
    PacketClientConnect* data = packet;

    add_peer(app->peers, data->client_id);

    return true;
}

bool handle_client_disconnect(App* app, void* packet) {
    PacketClientDisconnect* data = packet;

    remove_peer(app->peers, data->client_id);

    return true;
}

bool handle_client_list(App* app, void* packet) {
    PacketClientList* data = packet;

    for (size_t i = 0; i < data->list_len; ++i) {
        ReplicatedClientContext* ctx = &data->list[i];

        add_peer(app->peers, ctx->id);
        set_peer_display_name(app->peers, ctx->id, ctx->display_name);
    }

    print_peer_list(app->peers);

    return true;
}