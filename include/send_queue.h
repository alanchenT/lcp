#ifndef LCP_SEND_QUEUE_H
#define LCP_SEND_QUEUE_H

#include <pthread.h>

#include "common.h"

typedef struct Payload Payload;

struct Payload {
    char* data;
    size_t len;

    Payload* next;
};

typedef struct {
    Payload* head;
    Payload* tail;

    size_t count;

    pthread_mutex_t mutex;
    pthread_cond_t cond;

    bool is_shutdown;
} SendQueue;

SendQueue* init_send_queue();

Payload* reserve_send_queue_payload();

Payload* pop_send_queue(SendQueue* queue);
void shutdown_send_queue(SendQueue* queue);

#endif