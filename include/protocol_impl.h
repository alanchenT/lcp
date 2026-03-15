/*
    protocol_impl.h

    Macros for implementing packet serdes
*/

#ifndef LCP_PROTOCOL_IMPL_H
#define LCP_PROTOCOL_IMPL_H

#include <arpa/inet.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define WRITE_U8(field_name) \
    if (offset + 1 > buffer_size) { \
        return 0; \
    } \
    buffer[offset++] = packet->field_name;

#define WRITE_U16(field_name) \
    if (offset + 2 > buffer_size) { \
        return 0; \
    } \
    *(uint16_t*)(buffer + offset) = htons(packet->field_name); \
    offset += 2;

#define WRITE_U32(field_name) \
    if (offset + 4 > buffer_size) { \
        return 0; \
    } \
    *(uint32_t*)(buffer + offset) = htonl(packet->field_name); \
    offset += 4;

#define WRITE_STR(field_name) \
    { \
        size_t len = strlen(packet->field_name); \
        if (offset + 2 + len > buffer_size) { \
            return 0; \
        } \
        *(uint16_t*)(buffer + offset) = htons(len); \
        offset += 2; \
        memcpy(buffer + offset, packet->field_name, len); \
        offset += len; \
    }

#define READ_U8(field_name) \
    if (offset + 1 > buffer_size) { \
        return 0; \
    } \
    packet->field_name = buffer[offset++];

#define READ_U16(field_name) \
    if (offset + 2 > buffer_size) { \
        return 0; \
    } \
    packet->field_name = ntohs(*(uint16_t*)(buffer + offset)); \
    offset += 2;

#define READ_U32(field_name) \
    if (offset + 4 > buffer_size) { \
        return 0; \
    } \
    packet->field_name = ntohl(*(uint32_t*)(buffer + offset)); \
    offset += 4;

#define READ_STR(field_name) \
    { \
        if (offset + 2 > buffer_size) { \
            return 0; \
        } \
        size_t len = (size_t)ntohs(*(uint16_t*)(buffer + offset)); \
        if (len >= sizeof(packet->field_name) || offset + len > buffer_size) { \
            return 0; \
        } \
        offset += 2; \
        memcpy(packet->field_name, buffer + offset, len); \
        packet->field_name[len] = '\0'; \
        offset += len; \
    }

// clang-format off
#define IMPL_PACKET(name, snake_case_name, write_impl, read_impl) \
    size_t write_packet_##snake_case_name(const Packet##name* packet, uint8_t* buffer, size_t buffer_size) { \
        size_t offset = 0; \
        if (offset + 1 > buffer_size) { \
            return 0; \
        } \
        buffer[offset++] = (uint8_t)Packet##name##Id; \
        write_impl \
        return offset; \
    } \
    size_t read_packet_##snake_case_name(Packet##name* packet, const uint8_t* buffer, size_t buffer_size) { \
        size_t offset = 0; \
        read_impl \
        return offset; \
    }
// clang-format on

#endif