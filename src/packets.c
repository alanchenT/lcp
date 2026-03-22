#include "packets.h"

#include <stdlib.h>

#include "protocol_impl.h"

// clang-format off
IMPL_PACKET(ClientHandshake, client_handshake,
    SIZE_U8(client_id),

    WRITE_U8(client_id),

    READ_U8(client_id)
)

IMPL_PACKET(ClientConnect, client_connect, 
    SIZE_U8(client_id),

    WRITE_U8(client_id),
    
    READ_U8(client_id)
)

IMPL_PACKET(ClientDisconnect, client_disconnect,
    SIZE_U8(client_id),

    WRITE_U8(client_id),
    
    READ_U8(client_id)
)

IMPL_PACKET(ClientList, client_list,
    SIZE_ARRAY(ReplicatedClientContext, list,
        SIZE_STR(display_name)
        SIZE_U8(id)
        SIZE_BOOL(is_active)
    ),
    
    WRITE_ARRAY(ReplicatedClientContext, list, 
        WRITE_STR(display_name)
        WRITE_U8(id)
        WRITE_BOOL(is_active)
    ),
    
    READ_ARRAY(ReplicatedClientContext, list,
        READ_STR(display_name)
        READ_U8(id)
        READ_BOOL(is_active)
    )
)

IMPL_PACKET(ChatMessage, chat_message, 
    SIZE_U8(client_id)
    SIZE_STR(message),

    WRITE_U8(client_id) 
    WRITE_STR(message),

    READ_U8(client_id) 
    READ_STR(message)
)

IMPL_PACKET(SetDisplayName, set_display_name,
    SIZE_U8(client_id)
    SIZE_STR(display_name),

    WRITE_U8(client_id) 
    WRITE_STR(display_name),
    
    READ_U8(client_id) 
    READ_STR(display_name)
)
// clang-format on

PacketId get_packet_id(Packet* packet) {
    return packet->internal.packet_id;
}

static size_t sizeof_packet(const void* packet) {
    PacketId id = ((Packet*)packet)->internal.packet_id;

#define CASE(name, snake_case_name) \
    case Packet##name##Id: { \
        return size_packet_##snake_case_name(packet) + 1; \
    }

    switch (id) {
#define X(name, snake_case_name) CASE(name, snake_case_name)
        X_PACKETS
#undef X

        default: {
            return 0;
        }
    }

#undef CASE
}

size_t write_packet(void* packet) {
    PacketId id = ((Packet*)packet)->internal.packet_id;

#define CASE(name, snake_case_name) \
    case Packet##name##Id: { \
        Packet##name* cast_packet = packet; \
        size_t payload_len = sizeof_packet(cast_packet); \
        cast_packet->internal.payload = realloc(cast_packet->internal.payload, payload_len); \
        if (cast_packet->internal.payload == NULL) { \
            return 0; \
        } \
        cast_packet->internal.payload_len = payload_len; \
        return write_packet_##snake_case_name(cast_packet, cast_packet->internal.payload, payload_len); \
    }

    switch (id) {
#define X(name, snake_case_name) CASE(name, snake_case_name)
        X_PACKETS
#undef X

        default: {
            return 0;
        }
    }

#undef CASE
}

void* read_packet(const uint8_t* buffer, size_t buffer_size) {
    // First, read the packet ID to determine which read function to call
    PacketId id = buffer[0];

#define CASE(name, snake_case_name) \
    case Packet##name##Id: { \
        Packet##name* packet = malloc(sizeof(Packet##name)); \
        if (packet == NULL) { \
            return NULL; \
        } \
        size_t bytes = read_packet_##snake_case_name(packet, buffer + 1, buffer_size); \
        if (bytes == 0) { \
            free(packet); \
            return NULL; \
        } \
        packet->internal.payload_len = bytes; \
        packet->internal.payload = malloc(bytes); \
        packet->internal.packet_id = id; \
        packet->internal.next = NULL; \
        if (packet->internal.payload == NULL) { \
            free(packet); \
            return NULL; \
        } \
        memcpy(packet->internal.payload, buffer + 1, bytes); \
        return packet; \
    }

    switch (id) {
#define X(name, snake_case_name) CASE(name, snake_case_name)
        X_PACKETS
#undef X

        default: {
            return NULL;
        }
    }

#undef CASE
}

void* copy_packet(Packet* packet) {
    PacketId id = packet->internal.packet_id;

#define CASE(name, snake_case_name) \
    case Packet##name##Id: { \
        Packet##name* copy = malloc(sizeof(Packet##name)); \
        if (copy == NULL) { \
            return NULL; \
        } \
        Packet##name* casted = (Packet##name*)packet; \
        memcpy(copy, casted, sizeof(Packet##name)); \
        if (casted->internal.payload != NULL) { \
            copy->internal.payload = malloc(casted->internal.payload_len); \
            if (copy->internal.payload == NULL) { \
                free(copy); \
                return NULL; \
            } \
            memcpy(copy->internal.payload, casted->internal.payload, casted->internal.payload_len); \
        } \
        return copy; \
    }

    switch (id) {
#define X(name, snake_case_name) CASE(name, snake_case_name)
        X_PACKETS
#undef X

        default: {
            return NULL;
        }
    }

#undef CASE
}