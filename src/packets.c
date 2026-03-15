#include "packets.h"

#include <stdlib.h>

#include "protocol_impl.h"

// clang-format off
IMPL_PACKET(ClientConnect, client_connect, 
    WRITE_U8(client_id) 
    WRITE_STR(display_name),
    
    READ_U8(client_id) 
    READ_STR(display_name)
)

IMPL_PACKET(ChatMessage, chat_message, 
    WRITE_U8(client_id) 
    WRITE_STR(message),

    READ_U8(client_id) 
    READ_STR(message)
)
// clang-format on

size_t write_packet(const void* packet, uint8_t* buffer, size_t buffer_size) {
    const PacketDiscriminant* discriminant = packet;
    PacketId id = discriminant->packet_id;

#define CASE(name, snake_case_name) \
    case Packet##name##Id: { \
        return write_packet_##snake_case_name(packet, buffer, buffer_size); \
    }

    switch (id) {
        CASE(ClientConnect, client_connect)
        CASE(ChatMessage, chat_message)

        default: {
            return 0;
        }
    }

#undef CASE
}

void* read_packet(const uint8_t* buffer, size_t buffer_size, size_t* bytes_read, PacketId* packet_id) {
    // First, read the packet ID to determine which read function to call
    PacketId id = buffer[0];
    *bytes_read = 1;
    *packet_id = id;

#define CASE(name, snake_case_name) \
    case Packet##name##Id: { \
        Packet##name* packet = malloc(sizeof(Packet##name)); \
        if (packet == NULL) { \
            return NULL; \
        } \
        size_t bytes = read_packet_##snake_case_name(packet, buffer + 1, buffer_size); \
        if (bytes == 0) { \
            return NULL; \
        } \
        *bytes_read += bytes; \
        return packet; \
    }

    switch (id) {
        CASE(ClientConnect, client_connect)
        CASE(ChatMessage, chat_message)

        default: {
            return NULL;
        }
    }

#undef CASE
}