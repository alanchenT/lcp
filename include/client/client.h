#ifndef LCP_CLIENT_CLIENT_H
#define LCP_CLIENT_CLIENT_H

#include <pthread.h>

#include "common.h"

typedef struct Payload Payload;
typedef struct PayloadQueue PayloadQueue;

typedef struct Client {
    int socket_fd;

    pthread_t recv_thread;
    pthread_t send_thread;

    PayloadQueue* send_queue;
    PayloadQueue* recv_queue;

    bool is_connected;
} Client;

Client* alloc_client(void);

bool client_connect(Client* client);

bool client_disconnect(Client* client);

void client_send(Client* client, Payload* payload);

#endif