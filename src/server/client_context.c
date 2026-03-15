#include "server/client_context.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "net_shared.h"
#include "payload_queue.h"
#include "server/client_list.h"

static void* client_send_thread(void* arg) {
    ClientContext* ctx = arg;

    while (ctx->is_active) {
        Payload* payload = pop_payload_queue(ctx->send_queue);

        // The client disconnected
        if (payload == NULL) {
            break;
        }

        bool send_status = net_send(ctx->socket_fd, payload->data, payload->len);
        if (!send_status) {
            ctx->is_active = false;
            free_payload(payload);
            break;
        }

        free_payload(payload);
    }

    return NULL;
}

static void* client_recv_thread(void* arg) {
    ClientContext* ctx = arg;

    uint8_t recv_buffer[MAX_NET_BUFFER_SIZE];

    while (ctx->is_active) {
        size_t payload_len = net_recv(ctx->socket_fd, recv_buffer, sizeof(recv_buffer));
        if (payload_len == 0) { // Disconnected / error
            ctx->is_active = false;
            break;
        }

        push_payload_queue(ctx->recv_queue, recv_buffer, payload_len);
    }

    printf("bye bye\n");

    remove_client(ctx->parent_list, ctx);

    return NULL;
}

bool activate_client_context(ClientContext* ctx, int socket_fd) {
    ctx->socket_fd = socket_fd;
    ctx->send_queue = alloc_payload_queue();
    ctx->recv_queue = alloc_payload_queue();
    ctx->is_active = true;

    int pthread_status = 0;

    // Spawn threads to handle reading and writing
    pthread_status = pthread_create(&ctx->recv_thread, NULL, client_recv_thread, ctx);
    if (pthread_status != 0) {
        fprintf(stderr, "server: pthread_create failed (recv)\n");
        return false;
    }

    pthread_status = pthread_create(&ctx->send_thread, NULL, client_send_thread, ctx);
    if (pthread_status != 0) {
        fprintf(stderr, "server: pthread_create failed (send)\n");
        return false;
    }

    pthread_status = pthread_detach(ctx->recv_thread);
    if (pthread_status != 0) {
        fprintf(stderr, "server: pthread_detach failed (recv)\n");
        return false;
    }

    pthread_status = pthread_detach(ctx->send_thread);
    if (pthread_status != 0) {
        fprintf(stderr, "server: pthread_detach failed (send)\n");
        return false;
    }

    return true;
}

void shutdown_client_context(ClientContext* ctx) {
    shutdown_payload_queue(ctx->send_queue);
    close(ctx->socket_fd);
    ctx->is_active = false;
}