/*
    main.c (server)
*/

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "packet_queue.h"
#include "packets.h"
#include "server/client_list.h"
#include "server/net_callbacks.h"
#include "server/server.h"

static volatile sig_atomic_t is_running = true;

// static void signal_handler(int signal_number) {
//     printf("\nsignal: %s\n", strsignal(signal_number));

//     is_running = false;
// }

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

void update_client(ClientContext* ctx) {
    while (!is_packet_queue_empty(ctx->recv_queue)) {
        Packet* packet = poll_packet_queue(ctx->recv_queue);
        if (packet == NULL) {
            return;
        }

        switch (get_packet_id(packet)) {
            case PacketChatMessageId: {
                handle_chat_message(ctx, packet);
                break;
            }
            case PacketSetDisplayNameId: {
                handle_set_display_name(ctx, packet);
                break;
            }

            default: {
                break;
            }
        }

        free_packet(packet);
    }
}

int main() {
    Server* server = alloc_server();
    if (server == NULL) {
        fprintf(stderr, "server: failed to alloc server\n");
        exit(1);
    }

    if (!server_start(server)) {
        fprintf(stderr, "server: failed to start\n");
        free_server(server);
        exit(1);
    }

    // struct sigaction action;
    // action.sa_handler = signal_handler;
    // sigaction(SIGINT, &action, NULL);

    const double update_rate = 20.0;
    const double update_time = 1 / update_rate;

    //double last_update_time = get_time();

    while (server_is_running(server)) {
        double start_time = get_time();

        for_each_client(server->client_list, update_client);

        double delta_time = get_time() - start_time;
        if (delta_time < update_time) {
            usleep((update_time - delta_time) * 1e6);
        }

        //last_update_time = start_time;
    }

    free_server(server);

    return 0;
}