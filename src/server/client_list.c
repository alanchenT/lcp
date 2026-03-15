#include "server/client_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "net_shared.h"

ClientList* alloc_client_list(void) {
    ClientList* client_list = malloc(sizeof(ClientList));
    if (client_list == NULL) {
        return NULL;
    }

    client_list->count = 0;
    client_list->capacity = MAX_CLIENTS;
    pthread_mutex_init(&client_list->mutex, NULL);

    for (size_t i = 0; i < client_list->capacity; ++i) {
        ClientContext* ctx = &client_list->clients[i];
        ctx->id = i;
        ctx->is_active = false;
        ctx->parent_list = client_list;
    }

    return client_list;
}

void free_client_list(ClientList* list) {
    pthread_mutex_lock(&list->mutex);

    for (size_t i = 0; i < list->capacity; ++i) {
        ClientContext* ctx = &list->clients[i];
        free_payload_queue(ctx->recv_queue);
        free_payload_queue(ctx->send_queue);
    }

    pthread_mutex_unlock(&list->mutex);

    pthread_mutex_destroy(&list->mutex);
    free(list);
}

static ClientContext* reserve_client(ClientList* list) {
    pthread_mutex_lock(&list->mutex);

    // Reached max clients
    if (list->count + 1 > list->capacity) {
        pthread_mutex_unlock(&list->mutex);
        return NULL;
    }

    for (size_t i = 0; i < list->capacity; ++i) {
        ClientContext* ctx = &list->clients[i];
        if (!ctx->is_active) {
            ctx->is_active = true;
            list->count++;

            pthread_mutex_unlock(&list->mutex);
            return ctx;
        }
    }

    pthread_mutex_unlock(&list->mutex);
    return NULL;
}

bool add_client(ClientList* list, int socket_fd) {
    ClientContext* ctx = reserve_client(list);
    if (ctx == NULL) {
        return false;
    }

    return activate_client_context(ctx, socket_fd);
}

void remove_client(ClientList* list, ClientContext* ctx) {
    pthread_mutex_lock(&list->mutex);

    shutdown_client_context(ctx);
    list->count--;

    pthread_mutex_unlock(&list->mutex);
}

void remove_all_clients(ClientList* list) {
    pthread_mutex_lock(&list->mutex);

    for (size_t i = 0; i < list->capacity; ++i) {
        ClientContext* ctx = &list->clients[i];
        if (ctx->is_active) {
            shutdown_client_context(ctx);
        }
    }

    list->count = 0;

    pthread_mutex_unlock(&list->mutex);
}

void for_each_client(ClientList* list, void (*callback)(ClientContext*)) {
    for (size_t i = 0; i < list->capacity; ++i) {
        pthread_mutex_lock(&list->mutex);

        ClientContext* ctx = &list->clients[i];

        if (!ctx->is_active) {
            pthread_mutex_unlock(&list->mutex);
            continue;
        }

        callback(ctx);
        pthread_mutex_unlock(&list->mutex);
    }
}

void send_to_one(ClientContext* ctx, Payload* payload) {
    move_into_payload_queue(ctx->send_queue, payload);
}

void send_to_all(ClientList* list, Payload* payload) {
    for (size_t i = 0; i < list->capacity; ++i) {
        ClientContext* ctx = &list->clients[i];
        if (!ctx->is_active) {
            continue;
        }

        push_payload_queue(ctx->send_queue, payload->data, payload->len);
    }
}

void send_to_all_except(ClientList* list, ClientContext* exception, Payload* payload) {
    for (size_t i = 0; i < list->capacity; ++i) {
        ClientContext* ctx = &list->clients[i];
        if (!ctx->is_active) {
            continue;
        }

        if (ctx == exception) {
            continue;
        }

        push_payload_queue(ctx->send_queue, payload->data, payload->len);
    }
}