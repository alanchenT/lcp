#ifndef LCP_PACKETS_H
#define LCP_PACKETS_H

#include "common.h"
#include "protocol.h"

#define PACKET_ID_NAME(packet_name) Packet##packet_name##Id,

#define PACKETS \
    PACKET_ID_NAME(ClientConnect) \
    PACKET_ID_NAME(ChatMessage)

typedef enum { PACKETS } PacketId;

#define INIT_PACKET(type, var_name) \
    type var_name; \
    var_name.packet_id = type##Id;

/*
    Packet definitions
*/
BEGIN_PACKET_DECL(ClientConnect)
FIELD_U8(client_id)
FIELD_STR(display_name, CLIENT_NAME_MAX_LEN)
END_PACKET_DECL(ClientConnect, client_connect)

BEGIN_PACKET_DECL(ChatMessage)
FIELD_U8(client_id)
FIELD_STR(message, 256)
END_PACKET_DECL(ChatMessage, chat_message)

size_t write_packet(const void* packet, uint8_t* buffer, size_t buffer_size);

// The caller must `free` the returned packet.
void* read_packet(const uint8_t* buffer, size_t buffer_size, size_t* bytes_read, PacketId* packet_id);

#endif