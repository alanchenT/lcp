#ifndef LCP_PACKET_QUEUE_H
#define LCP_PACKET_QUEUE_H

#include <pthread.h>

#include "common.h"

typedef struct Packet Packet;

typedef struct PacketQueue {
    Packet* head;
    Packet* tail;

    size_t count;

    pthread_mutex_t mutex;
    pthread_cond_t cond;

    bool is_shutdown;
} PacketQueue;

PacketQueue* alloc_packet_queue(void);

void free_packet_queue(PacketQueue* queue);

void free_packet(void* packet);

/*
    Moves the constructed packet into the queue. The queue will take responsibility for freeing the packet.
    It's assumed that you populated all of its fields prior to calling this.
*/
bool write_packet_queue(PacketQueue* queue, Packet* packet);

bool recv_packet_queue(PacketQueue* queue, const uint8_t* buffer, size_t buffer_size);

void* pop_packet_queue(PacketQueue* queue);

// A non-blocking variant of `pop_payload_queue` that returns `NULL` when the queue is empty
void* poll_packet_queue(PacketQueue* queue);

bool is_packet_queue_empty(PacketQueue* queue);

void shutdown_packet_queue(PacketQueue* queue);

#endif