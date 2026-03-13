#include "server/client_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "net_shared.h"

ClientList* init_client_list(void) {
    ClientList* client_list = malloc(sizeof(ClientList));
    if (client_list == nullptr) {
        perror("server: unable to malloc ClientList");
        return nullptr;
    }

    client_list->count = 0;
    client_list->capacity = MAX_CLIENTS;
    pthread_mutex_init(&client_list->mutex, nullptr);

    for (size_t i = 0; i < client_list->capacity; ++i) {
        ClientContext* ctx = &client_list->clients[i];
        ctx->id = i;
        ctx->is_active = false;
    }

    return client_list;
}

static ClientContext* reserve_client(ClientList* list) {
    pthread_mutex_lock(&list->mutex);

    if (list->count + 1 > list->capacity) {
        pthread_mutex_unlock(&list->mutex);
        return nullptr;
    }

    for (size_t i = 0; i < list->capacity; ++i) {
        ClientContext* ctx = &list->clients[i];
        if (!ctx->is_reserved) { // Mark as reserved
            ctx->is_reserved = true;
            list->count++;

            pthread_mutex_unlock(&list->mutex);
            return ctx;
        }
    }

    pthread_mutex_unlock(&list->mutex);
    return nullptr;
}

bool drop_client(ClientList* list, ClientContext* ctx) {
    pthread_mutex_lock(&list->mutex);

    close(ctx->socket_fd);
    ctx->is_active = false;
    ctx->is_reserved = false;

    list->count--;

    pthread_mutex_unlock(&list->mutex);

    return true;
}

void* client_send_thread(void* arg) {
    // ClientContext* ctx = arg;

    return nullptr;
}

void* client_recv_thread(void* arg) {
    ClientContext* ctx = arg;

    printf("recv thread\n");

    char recv_buffer[MAX_NET_BUFFER_SIZE];

    while (ctx->is_active) {
        bool recv_status = net_recv(ctx->socket_fd, recv_buffer, sizeof(recv_buffer));
        if (!recv_status) { // Disconnected / error
            ctx->is_active = false;
            break;
        }

        printf("received: %s\n", recv_buffer);
    }

    printf("bye bye\n");

    shutdown_send_queue(ctx->send_queue);

    return nullptr;
}

bool add_client(ClientList* list, int socket_fd) {
    ClientContext* ctx = reserve_client(list);
    if (ctx == nullptr) {
        return false;
    }

    ctx->socket_fd = socket_fd;
    ctx->send_queue = init_send_queue();
    ctx->is_active = true;
    ctx->parent_list = list;

    int pthread_status = 0;

    // Spawn threads to handle reading and writing
    pthread_status = pthread_create(&ctx->recv_thread, nullptr, client_recv_thread, ctx);
    if (pthread_status != 0) {
        fprintf(stderr, "server: pthread_create failed\n");
        return false;
    }

    pthread_status = pthread_create(&ctx->send_thread, nullptr, client_send_thread, ctx);
    if (pthread_status != 0) {
        fprintf(stderr, "server: pthread_create failed\n");
        return false;
    }

    pthread_status = pthread_detach(ctx->recv_thread);
    if (pthread_status != 0) {
        fprintf(stderr, "server: pthread_detach failed\n");
        return false;
    }

    pthread_status = pthread_detach(ctx->send_thread);
    if (pthread_status != 0) {
        fprintf(stderr, "server: pthread_detach failed\n");
        return false;
    }

    return true;
}