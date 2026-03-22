#ifndef LCP_PACKETS_H
#define LCP_PACKETS_H

#include <stdlib.h>

#include "common.h"
#include "protocol.h"

#define PACKET_ID(packet_type_name) packet_type_name##Id

#define ALLOC_PACKET(type, var_name) \
    type* var_name = malloc(sizeof(type)); \
    var_name->internal.packet_id = type##Id; \
    var_name->internal.payload = NULL; \
    var_name->internal.payload_len = 0;

#define X_PACKETS \
    X(ClientHandshake, client_handshake) \
    X(ClientConnect, client_connect) \
    X(ClientDisconnect, client_disconnect) \
    X(ClientList, client_list) \
    X(ChatMessage, chat_message) \
    X(SetDisplayName, set_display_name)

// clang-format off
typedef enum {

#define X(packet_name, _) Packet##packet_name##Id,
    X_PACKETS
#undef X

    MAX_PACKET_ID,

} PacketId;
// clang-format on

/*
    Packet definitions
*/

// clang-format off
BEGIN_PACKET_DECL(ClientHandshake)
    FIELD_U8(client_id)
END_PACKET_DECL(ClientHandshake, client_handshake)

BEGIN_PACKET_DECL(ClientConnect)
    FIELD_U8(client_id)
END_PACKET_DECL(ClientConnect, client_connect)

BEGIN_PACKET_DECL(ClientDisconnect)
    FIELD_U8(client_id)
END_PACKET_DECL(ClientDisconnect, client_disconnect)

/*
    ClientList: 
*/
typedef struct ReplicatedClientContext {
    char display_name[CLIENT_DISPLAY_NAME_MAX_LEN];
    size_t id;
    bool is_active;
} ReplicatedClientContext;

BEGIN_PACKET_DECL(ClientList)
    FIELD_ARRAY(ReplicatedClientContext, list, MAX_CLIENTS)
END_PACKET_DECL(ClientList, client_list)

BEGIN_PACKET_DECL(ChatMessage)
    FIELD_U8(client_id)
    FIELD_STR(message, 256)
END_PACKET_DECL(ChatMessage, chat_message)

BEGIN_PACKET_DECL(SetDisplayName)
    FIELD_U8(client_id)
    FIELD_STR(display_name, CLIENT_DISPLAY_NAME_MAX_LEN)
END_PACKET_DECL(SetDisplayName, set_display_name)
// clang-format on

PacketId get_packet_id(Packet* packet);

size_t write_packet(void* packet);

// The caller must `free` the returned packet.
void* read_packet(const uint8_t* buffer, size_t buffer_size);

void* copy_packet(Packet* packet);

#endif