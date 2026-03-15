#ifndef LCP_SERVER_CLIEINT_CONTEXT_H
#define LCP_SERVER_CLIEINT_CONTEXT_H

#include <pthread.h>

#include "common.h"

typedef struct ClientList ClientList;
typedef struct PayloadQueue PayloadQueue;

typedef struct ClientContext {
    char display_name[CLIENT_NAME_MAX_LEN];
    int socket_fd;
    size_t id;

    pthread_t recv_thread;
    pthread_t send_thread;

    PayloadQueue* recv_queue;
    PayloadQueue* send_queue;

    bool is_active;

    ClientList* parent_list;
} ClientContext;

bool activate_client_context(ClientContext* ctx, int socket_fd);

void shutdown_client_context(ClientContext* ctx);

#endif