#ifndef LCP_NET_SHARED_H
#define LCP_NET_SHARED_H

#include <stdint.h>
#include <sys/socket.h>

// Returns the Internet address of the socket address
void* extract_sockaddr(struct sockaddr* addr);

bool net_send(int socket_fd, const void* payload, size_t payload_len);

bool net_recv(int socket_fd, void* recv_buffer, size_t recv_buffer_size);

#endif