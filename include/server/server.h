#ifndef LCP_SERVER_SERVER_H
#define LCP_SERVER_SERVER_H

#include <pthread.h>

#include "common.h"

typedef struct ClientList ClientList;

typedef struct Server {
    ClientList* client_list;
    pthread_t accept_thread;

    int listener_socket_fd;
    bool is_running;
    pthread_mutex_t is_running_mutex;
} Server;

Server* alloc_server(void);
void free_server(Server* server);

bool server_start(Server* server);

void server_stop(Server* server);

bool server_is_running(Server* server);

#endif