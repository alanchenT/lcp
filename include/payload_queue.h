#ifndef LCP_PAYLOAD_QUEUE_H
#define LCP_PAYLOAD_QUEUE_H

#include <pthread.h>

#include "common.h"

typedef struct Payload Payload;

struct Payload {
    uint8_t* data;
    size_t len;

    Payload* next;
};

typedef struct PayloadQueue {
    Payload* head;
    Payload* tail;

    size_t count;

    pthread_mutex_t mutex;
    pthread_cond_t cond;

    bool is_shutdown;
} PayloadQueue;

PayloadQueue* alloc_payload_queue(void);
void free_payload_queue(PayloadQueue* queue);

Payload* alloc_payload(size_t len);
void free_payload(Payload* payload);

// Moves the Payload into the queue. The queue will assume responsibility of freeing
void move_into_payload_queue(PayloadQueue* queue, Payload* payload);

// Constructs a Payload with the data and len.
void push_payload_queue(PayloadQueue* queue, const uint8_t* data, size_t len);

Payload* pop_payload_queue(PayloadQueue* queue);

// A non-blocking variant of `pop_payload_queue` that returns `NULL` when the queue is empty
Payload* poll_payload_queue(PayloadQueue* queue);

bool is_payload_queue_empty(PayloadQueue* queue);

void shutdown_payload_queue(PayloadQueue* queue);

#endif