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

/*
    Size macros
*/
#define SIZE_U8(field_name) total_size += sizeof(uint8_t);
#define SIZE_U16(field_name) total_size += sizeof(uint16_t);
#define SIZE_U32(field_name) total_size += sizeof(uint32_t);
#define SIZE_BOOL(field_name) total_size += sizeof(uint8_t);

#define SIZE_STR(field_name) total_size += sizeof(uint16_t) + strlen(packet->field_name);
#define SIZE_ARRAY(type, field_name, element_size) \
    { \
        total_size += sizeof(uint16_t); \
        for (size_t i = 0; i < packet->field_name##_len; ++i) { \
            const type* _ = &packet->field_name[i]; \
            const type* packet = _; \
            element_size \
        } \
    }
#define SIZE_STRUCT(field_name, field_sizes) total_size += field_sizes;

/*
    Write macros
*/
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

#define WRITE_BOOL(field_name) WRITE_U8(field_name)

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

#define WRITE_ARRAY(type, field_name, write_impl) \
    { \
        size_t len = packet->field_name##_len; \
        if (offset + 2 > buffer_size) { \
            return 0; \
        } \
        *(uint16_t*)(buffer + offset) = htons(len); \
        offset += 2; \
        for (size_t i = 0; i < len; ++i) { \
            const type* _ = &packet->field_name[i]; \
            const type* packet = _; \
            write_impl \
        } \
    }

#define WRITE_STRUCT(type, field_name, write_impl) \
    { \
        const type* _ = &packet->field_name; \
        const type* packet = _; \
        write_impl \
    }

/*
    Read macros
*/
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

#define READ_BOOL(field_name) READ_U8(field_name)

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

#define READ_ARRAY(type, field_name, read_impl) \
    { \
        if (offset + 2 > buffer_size) { \
            return 0; \
        } \
        size_t len = (size_t)ntohs(*(uint16_t*)(buffer + offset)); \
        packet->field_name##_len = len; \
        offset += 2; \
        for (size_t i = 0; i < len; ++i) { \
            type* _ = &packet->field_name[i]; \
            type* packet = _; \
            read_impl \
        } \
    }

#define READ_STRUCT(type, field_name, read_impl) \
    { \
        type* _ = &packet->field_name; \
        type* packet = _; \
        read_impl \
    }

// clang-format off
#define IMPL_PACKET(name, snake_case_name, size_impl, write_impl, read_impl) \
    size_t size_packet_##snake_case_name(const Packet##name* packet) { \
        (void)packet; \
        size_t total_size = 0; \
        size_impl \
        return total_size; \
    } \
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