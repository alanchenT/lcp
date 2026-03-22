#ifndef LCP_SERVER_CLIEINT_CONTEXT_H
#define LCP_SERVER_CLIEINT_CONTEXT_H

#include <pthread.h>

#include "common.h"

typedef struct ClientList ClientList;
typedef struct PacketQueue PacketQueue;

typedef struct ClientContext {
    char display_name[CLIENT_DISPLAY_NAME_MAX_LEN];
    size_t id;
    bool is_active;

    int socket_fd;

    pthread_t recv_thread;
    pthread_t send_thread;

    PacketQueue* recv_queue;
    PacketQueue* send_queue;

    ClientList* parent_list;
} ClientContext;

bool activate_client_context(ClientContext* ctx, int socket_fd);

void shutdown_client_context(ClientContext* ctx);

void set_client_context_display_name(ClientContext* ctx, const char* display_name);

#endif