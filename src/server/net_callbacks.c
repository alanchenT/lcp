#include "server/net_callbacks.h"

#include <stdio.h>
#include <string.h>

#include "packet_queue.h"
#include "packets.h"
#include "server/client_context.h"
#include "server/client_list.h"

bool handle_chat_message(ClientContext* ctx, void* packet) {
    PacketChatMessage* chat_message = packet;
    chat_message->client_id = ctx->id;
    printf("%d said: %s\n", chat_message->client_id, chat_message->message);

    // TODO: send_to_all_except
    send_to_all(ctx->parent_list, copy_packet(packet));

    return true;
}

bool handle_set_display_name(ClientContext* ctx, void* packet) {
    PacketSetDisplayName* display_name = packet;
    display_name->client_id = ctx->id;
    set_client_context_display_name(ctx, display_name->display_name);

    // TODO: send_to_all_except
    send_to_all(ctx->parent_list, copy_packet(packet));

    return true;
}

static PacketClientList* serialize_client_list(ClientList* list) {
    ALLOC_PACKET(PacketClientList, packet);
    packet->list_len = list->count;

    size_t current_index = 0;
    for (size_t i = 0; i < list->capacity; ++i) {
        ClientContext* ctx = &list->clients[i];
        if (!ctx->is_active) {
            continue;
        }

        packet->list[current_index].id = ctx->id;
        packet->list[current_index].is_active = true;
        strncpy(packet->list[current_index].display_name, ctx->display_name, CLIENT_DISPLAY_NAME_MAX_LEN);

        current_index++;
    }

    return packet;
}

bool handle_client_connect(ClientContext* ctx) {
    ALLOC_PACKET(PacketClientConnect, packet);
    packet->client_id = ctx->id;

    // Inform other clients of the new guy
    send_to_all_except(ctx->parent_list, ctx, packet);

    // Inform the new guy their ID
    ALLOC_PACKET(PacketClientHandshake, handshake_packet);
    handshake_packet->client_id = ctx->id;
    send_to_one(ctx, handshake_packet);

    // Send the client list to the new guy
    Packet* list_packet = (void*)serialize_client_list(ctx->parent_list);
    send_to_one(ctx, list_packet);

    return true;
}

bool handle_client_disconnect(ClientContext* ctx) {
    ALLOC_PACKET(PacketClientDisconnect, packet);
    packet->client_id = ctx->id;

    send_to_all_except(ctx->parent_list, ctx, packet);

    return true;
}