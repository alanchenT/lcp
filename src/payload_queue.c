#include "payload_queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PayloadQueue* alloc_payload_queue(void) {
    PayloadQueue* queue = malloc(sizeof(PayloadQueue));
    if (queue == NULL) {
        perror("[alloc_payload_queue] malloc error");
        return NULL;
    }

    queue->head = NULL;
    queue->count = 0;
    queue->is_shutdown = false;

    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);

    return queue;
}

// This does NOT lock/unlock the mutex!
static void free_all_payloads(PayloadQueue* queue) {
    Payload* current = queue->head;
    while (current != NULL) {
        Payload* next = current->next;
        free(current);
        current = next;
    }

    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;
}

void free_payload_queue(PayloadQueue* queue) {
    pthread_mutex_lock(&queue->mutex);

    free_all_payloads(queue);
    queue->is_shutdown = true;

    pthread_mutex_unlock(&queue->mutex);

    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);

    free(queue);
}

Payload* alloc_payload(size_t len) {
    Payload* new_payload = malloc(sizeof(Payload));
    if (new_payload == NULL) {
        perror("[alloc_payload] malloc error");
        return NULL;
    }

    new_payload->data = malloc(sizeof(char) * len);
    if (new_payload->data == NULL) {
        perror("[alloc_payload] payload->data malloc error");
        return NULL;
    }

    new_payload->len = len;
    new_payload->next = NULL;

    return new_payload;
}

void free_payload(Payload* payload) {
    payload->next = NULL;
    free(payload->data);
    free(payload);
}

void move_into_payload_queue(PayloadQueue* queue, Payload* payload) {
    pthread_mutex_lock(&queue->mutex);

    if (queue->is_shutdown) {
        pthread_mutex_unlock(&queue->mutex);
        free(payload);

        return;
    }

    if (queue->head == NULL) {
        queue->tail = payload;
    } else {
        payload->next = queue->head;
    }
    queue->head = payload;
    queue->count++;

    // Wake up the thread
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
}

void push_payload_queue(PayloadQueue* queue, const uint8_t* data, size_t len) {
    Payload* new_payload = alloc_payload(len);
    memcpy(new_payload->data, data, len);

    move_into_payload_queue(queue, new_payload);
}

Payload* pop_payload_queue(PayloadQueue* queue) {
    pthread_mutex_lock(&queue->mutex);

    // Block until there's a payload or the queue gets shutdown
    while (queue->head == NULL && !queue->is_shutdown) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }

    if (queue->is_shutdown) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }

    Payload* payload = queue->head;
    queue->head = payload->next;
    if (queue->head == NULL) {
        queue->tail = NULL;
    }
    queue->count--;

    pthread_mutex_unlock(&queue->mutex);

    return payload;
}

Payload* poll_payload_queue(PayloadQueue* queue) {
    pthread_mutex_lock(&queue->mutex);

    if (queue->head == NULL) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }

    Payload* payload = queue->head;
    queue->head = payload->next;
    if (queue->head == NULL) {
        queue->tail = NULL;
    }
    queue->count--;

    pthread_mutex_unlock(&queue->mutex);

    return payload;
}

bool is_payload_queue_empty(PayloadQueue* queue) {
    pthread_mutex_lock(&queue->mutex);
    bool is_empty = queue->count == 0;
    pthread_mutex_unlock(&queue->mutex);

    return is_empty;
}

void shutdown_payload_queue(PayloadQueue* queue) {
    pthread_mutex_lock(&queue->mutex);

    queue->is_shutdown = true;
    free_all_payloads(queue);

    pthread_cond_broadcast(&queue->cond);

    pthread_mutex_unlock(&queue->mutex);
}