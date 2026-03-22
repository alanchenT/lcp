#ifndef LCP_CLIENT_CLIENT_H
#define LCP_CLIENT_CLIENT_H

#include <pthread.h>

#include "common.h"

typedef struct Packet Packet;
typedef struct PacketQueue PacketQueue;
typedef struct PeerList PeerList;

typedef struct Client {
    char display_name[CLIENT_DISPLAY_NAME_MAX_LEN];
    int socket_fd;

    int8_t id;

    pthread_t recv_thread;
    pthread_t send_thread;

    PacketQueue* send_queue;
    PacketQueue* recv_queue;

    bool is_connected;

    PeerList* peers;
} Client;

Client* alloc_client(void);
void free_client(Client* client);

bool client_completed_handshake(Client* client);

bool client_connect(Client* client);

bool client_disconnect(Client* client);

void client_send(Client* client, void* packet);

#endif