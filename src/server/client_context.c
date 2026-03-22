#include "server/client_context.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "net_shared.h"
#include "packet_queue.h"
#include "packets.h"
#include "server/client_list.h"
#include "server/net_callbacks.h"

static void* client_send_thread(void* arg) {
    ClientContext* ctx = arg;

    while (ctx->is_active) {
        Packet* packet = pop_packet_queue(ctx->send_queue);

        // The client disconnected
        if (packet == NULL) {
            break;
        }

        bool send_status = net_send(ctx->socket_fd, packet->internal.payload, packet->internal.payload_len);
        if (!send_status) {
            free_packet(packet);
            break;
        }

        free_packet(packet);
    }

    ctx->is_active = false;
    return NULL;
}

static void* client_recv_thread(void* arg) {
    ClientContext* ctx = arg;

    uint8_t recv_buffer[MAX_NET_BUFFER_SIZE];

    while (ctx->is_active) {
        size_t payload_len = net_recv(ctx->socket_fd, recv_buffer, sizeof(recv_buffer));
        if (payload_len == 0) { // Disconnected / error
            break;
        }

        if (!recv_packet_queue(ctx->recv_queue, recv_buffer, payload_len)) {
            break;
        }
    }

    printf("client disconnect\n");

    handle_client_disconnect(ctx);

    remove_client(ctx->parent_list, ctx);

    return NULL;
}

bool activate_client_context(ClientContext* ctx, int socket_fd) {
    ctx->socket_fd = socket_fd;
    ctx->send_queue = alloc_packet_queue();
    ctx->recv_queue = alloc_packet_queue();
    ctx->is_active = true;

    memset(ctx->display_name, 0, sizeof(ctx->display_name));

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
    shutdown_packet_queue(ctx->send_queue);
    close(ctx->socket_fd);
    ctx->is_active = false;
}

void set_client_context_display_name(ClientContext* ctx, const char* display_name) {
    strncpy(ctx->display_name, display_name, CLIENT_DISPLAY_NAME_MAX_LEN);
}