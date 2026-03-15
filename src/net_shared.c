#include "net_shared.h"

#include <netdb.h>
#include <stdio.h>

typedef struct {
    uint32_t payload_len;
} PayloadHeader;

void* extract_sockaddr(struct sockaddr* addr) {
    if (addr->sa_family == AF_INET) {
        struct sockaddr_in* ipv4_addr = (struct sockaddr_in*)addr;
        return &ipv4_addr->sin_addr;
    } else {
        struct sockaddr_in6* ipv6_addr = (struct sockaddr_in6*)addr;
        return &ipv6_addr->sin6_addr;
    }
}

static bool send_all(int socket_fd, const void* payload, size_t payload_len) {
    size_t current_bytes_sent = 0;

    while (current_bytes_sent < payload_len) {
        ssize_t bytes_sent =
            send(socket_fd, (const uint8_t*)payload + current_bytes_sent, payload_len - current_bytes_sent, 0);

        if (bytes_sent <= 0) {
            return false;
        }

        current_bytes_sent += bytes_sent;
    }

    return true;
}

bool net_send(int socket_fd, const void* payload, size_t payload_len) {
    PayloadHeader header;
    header.payload_len = htonl(payload_len);

    if (!send_all(socket_fd, &header, sizeof(header))) {
        return false;
    }

    if (!send_all(socket_fd, payload, payload_len)) {
        return false;
    }

    return true;
}

static bool recv_all(int socket_fd, void* recv_buffer, size_t payload_len) {
    size_t current_bytes_read = 0;

    while (current_bytes_read < payload_len) {
        ssize_t bytes_read =
            recv(socket_fd, (uint8_t*)recv_buffer + current_bytes_read, payload_len - current_bytes_read, 0);

        if (bytes_read <= 0) {
            return false;
        }

        current_bytes_read += bytes_read;
    }

    return true;
}

size_t net_recv(int socket_fd, void* recv_buffer, size_t recv_buffer_size) {
    PayloadHeader header;
    if (!recv_all(socket_fd, &header, sizeof(header))) {
        return 0;
    }

    uint32_t payload_len = ntohl(header.payload_len);

    if (payload_len > recv_buffer_size) { // The payload is too large!
        return 0;
    }

    if (!recv_all(socket_fd, recv_buffer, payload_len)) {
        return 0;
    }

    return payload_len;
}