#ifndef LCP_SERVER_CLIENT_LIST_H
#define LCP_SERVER_CLIENT_LIST_H

#include <pthread.h>

#include "common.h"
#include "server/client_context.h" // We need to know the size of ClientContext

typedef struct Packet Packet;

typedef struct ClientList {
    ClientContext clients[MAX_CLIENTS];
    size_t count;
    size_t capacity;
    pthread_mutex_t mutex;
} ClientList;

ClientList* alloc_client_list(void);
void free_client_list(ClientList* list);

ClientContext* add_client(ClientList* list, int socket_fd);

void remove_client(ClientList* list, ClientContext* ctx);
void remove_all_clients(ClientList* list);

void for_each_client(ClientList* list, void (*callback)(ClientContext*));

void send_to_one(ClientContext* ctx, void* packet);
void send_to_all(ClientList* list, void* packet);
void send_to_all_except(ClientList* list, ClientContext* exception, void* packet);

#endif