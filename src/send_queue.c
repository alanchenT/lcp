#include "send_queue.h"

#include <stdio.h>
#include <stdlib.h>

SendQueue* init_send_queue() {
    SendQueue* queue = malloc(sizeof(SendQueue));
    if (queue == nullptr) {
        perror("[init_send_queue] malloc error");
        return nullptr;
    }

    queue->head = nullptr;
    queue->count = 0;
    queue->is_shutdown = false;

    pthread_mutex_init(&queue->mutex, nullptr);
    pthread_cond_init(&queue->cond, nullptr);

    return queue;
}

void push_send_queue(SendQueue* queue, const char* data, size_t len) {}

Payload* pop_send_queue(SendQueue* queue) {
    pthread_mutex_lock(&queue->mutex);

    // Block until there's a payload or the queue gets shutdown
    while (queue->head == nullptr || !queue->is_shutdown) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }

    if (queue->is_shutdown) {
        pthread_mutex_unlock(&queue->mutex);
        return nullptr;
    }

    Payload* payload = queue->head;
    queue->head = payload->next;
    if (queue->head == nullptr) {
        queue->tail = nullptr;
    }
    queue->count--;

    pthread_mutex_unlock(&queue->mutex);

    return payload;
}

void shutdown_send_queue(SendQueue* queue) {
    pthread_mutex_lock(&queue->mutex);

    queue->is_shutdown = true;
    pthread_cond_broadcast(&queue->cond);

    pthread_mutex_unlock(&queue->mutex);
}