#include "packet_queue.h"

#include <stdio.h>
#include <stdlib.h>

#include "packets.h"

PacketQueue* alloc_packet_queue(void) {
    PacketQueue* queue = malloc(sizeof(PacketQueue));
    if (queue == NULL) {
        perror("[alloc_packet_queue] malloc error");
        return NULL;
    }

    queue->head = NULL;
    queue->count = 0;
    queue->is_shutdown = false;

    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);

    return queue;
}

void free_packet(void* packet) {
    PacketInternal* internal = &((Packet*)packet)->internal;
    if (internal->payload != NULL) {
        free(internal->payload);
        internal->payload_len = 0;
    }

    free(packet);
}

// This does NOT lock/unlock the mutex!
static void free_all_packets(PacketQueue* queue) {
    Packet* current = queue->head;
    while (current != NULL) {
        Packet* next = current->internal.next;
        free_packet(current);
        current = next;
    }

    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;
}

void free_packet_queue(PacketQueue* queue) {
    pthread_mutex_lock(&queue->mutex);

    free_all_packets(queue);
    queue->is_shutdown = true;

    pthread_mutex_unlock(&queue->mutex);

    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);

    free(queue);
}

static void push_packet_queue(PacketQueue* queue, Packet* packet) {
    pthread_mutex_lock(&queue->mutex);

    if (queue->is_shutdown) {
        pthread_mutex_unlock(&queue->mutex);
        free_packet(packet);

        return;
    }

    packet->internal.next = NULL;
    if (queue->head == NULL) {
        queue->head = packet;
    } else {
        queue->tail->internal.next = packet;
    }
    queue->tail = packet;

    queue->count++;

    // Wake up the thread
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
}

bool write_packet_queue(PacketQueue* queue, Packet* packet) {
    size_t bytes_written = write_packet(packet);
    if (bytes_written == 0) {
        return false;
    }

    push_packet_queue(queue, packet);

    return true;
}

bool recv_packet_queue(PacketQueue* queue, const uint8_t* buffer, size_t buffer_size) {
    Packet* packet = read_packet(buffer, buffer_size);
    if (packet == NULL) {
        return false;
    }

    push_packet_queue(queue, packet);

    return true;
}

void* pop_packet_queue(PacketQueue* queue) {
    pthread_mutex_lock(&queue->mutex);

    // Block until there's a payload or the queue gets shutdown
    while (queue->head == NULL && !queue->is_shutdown) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }

    if (queue->is_shutdown) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }

    Packet* packet = queue->head;
    queue->head = packet->internal.next;
    if (queue->head == NULL) {
        queue->tail = NULL;
    }
    queue->count--;

    pthread_mutex_unlock(&queue->mutex);

    return packet;
}

// A non-blocking variant of `pop_payload_queue` that returns `NULL` when the queue is empty
void* poll_packet_queue(PacketQueue* queue) {
    pthread_mutex_lock(&queue->mutex);

    if (queue->head == NULL) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }

    Packet* packet = queue->head;
    queue->head = packet->internal.next;
    if (queue->head == NULL) {
        queue->tail = NULL;
    }
    queue->count--;

    pthread_mutex_unlock(&queue->mutex);

    return packet;
}

bool is_packet_queue_empty(PacketQueue* queue) {
    pthread_mutex_lock(&queue->mutex);
    bool is_empty = queue->count == 0;
    pthread_mutex_unlock(&queue->mutex);

    return is_empty;
}

void shutdown_packet_queue(PacketQueue* queue) {
    pthread_mutex_lock(&queue->mutex);

    queue->is_shutdown = true;
    free_all_packets(queue);

    pthread_cond_broadcast(&queue->cond);

    pthread_mutex_unlock(&queue->mutex);
}