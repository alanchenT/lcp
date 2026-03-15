/*
    protocol.h

    Macros for defining packets
*/

#ifndef LCP_PROTOCOL_H
#define LCP_PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t packet_id;
} PacketDiscriminant;

// clang-format off
#define BEGIN_PACKET_DECL(name) \
    typedef struct Packet##name { \
        uint8_t packet_id;

#define END_PACKET_DECL(name, snake_case_name) \
    } Packet##name; \
    size_t write_packet_##snake_case_name(const Packet##name* packet, uint8_t* buffer, size_t buffer_size); \
    size_t read_packet_##snake_case_name(Packet##name* packet, const uint8_t* buffer, size_t buffer_size);
// clang-format on

// Field definitions
#define FIELD_U8(field_name) uint8_t field_name;
#define FIELD_U16(field_name) uint16_t field_name;
#define FIELD_U32(field_name) uint32_t field_name;

#define FIELD_I8(field_name) int8_t field_name;
#define FIELD_I16(field_name) int16_t field_name;
#define FIELD_I32(field_name) int32_t field_name;

#define FIELD_STR(field_name, max_len) char field_name[max_len];

#endif