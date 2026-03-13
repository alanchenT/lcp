#ifndef LCP_SERVER_CLIENT_LIST_H
#define LCP_SERVER_CLIENT_LIST_H

#include <pthread.h>

#include "common.h"
#include "send_queue.h"

typedef struct ClientList ClientList;

typedef struct {
    char display_name[CLIENT_NAME_MAX_LEN];
    int socket_fd;
    size_t id;

    pthread_t recv_thread;
    pthread_t send_thread;
    SendQueue* send_queue;

    bool is_active;
    bool is_reserved;

    ClientList* parent_list;
} ClientContext;

struct ClientList {
    ClientContext clients[MAX_CLIENTS];
    size_t count;
    size_t capacity;
    pthread_mutex_t mutex;
};

typedef struct {
    char display_name[CLIENT_NAME_MAX_LEN];

    int fd; // The socket file descriptor
    size_t id; // Index in `clients`
} Client;

ClientList* init_client_list(void);

bool add_client(ClientList* list, int socket_fd);

#endif