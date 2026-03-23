#ifndef LCP_COMMON_H
#define LCP_COMMON_H

// Common type definitions
#include <stdbool.h> // true, false
#include <stdint.h> // sized integer types
#include <sys/types.h>

// The port to which the server is bound and clients will connect
#define SERVER_PORT "6967"
#define SERVER_LISTENER_BACKLOG 20

#define CLIENT_DISPLAY_NAME_MAX_LEN 32
#define CLIENT_DISPLAY_NAME_MIN_LEN 3

#define MAX_CLIENTS 24

#define MAX_NET_BUFFER_SIZE 4096

#endif