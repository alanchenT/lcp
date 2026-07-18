#include "client/peer_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PeerList* alloc_peer_list() {
    PeerList* new_list = malloc(sizeof(PeerList));
    if (new_list == NULL) {
        return NULL;
    }

    new_list->count = 0;
    new_list->capacity = MAX_CLIENTS;

    for (size_t i = 0; i < new_list->capacity; ++i) {
        PeerInfo* peer = &new_list->peers[i];
        peer->id = i;
        peer->is_active = false;

        memset(peer->display_name, 0, sizeof(peer->display_name));
    }

    return new_list;
}

void free_peer_list(PeerList* list) {
    free(list);
}

void add_peer(PeerList* list, size_t id) {
    PeerInfo* peer = &list->peers[id];
    peer->is_active = true;

    list->count++;
}

void remove_peer(PeerList* list, size_t id) {
    PeerInfo* peer = &list->peers[id];
    peer->is_active = false;
    memset(peer->display_name, 0, sizeof(peer->display_name));

    list->count--;
}

const char* get_peer_display_name(PeerList* list, size_t id) {
    return list->peers[id].display_name;
}

void set_peer_display_name(PeerList* list, size_t id, const char* display_name) {
    PeerInfo* peer = &list->peers[id];
    strncpy(peer->display_name, display_name, CLIENT_DISPLAY_NAME_MAX_LEN);
}

void reset_peer_list(PeerList* list) {
    list->count = 0;

    for (size_t i = 0; i < list->capacity; ++i) {
        PeerInfo* peer = &list->peers[i];
        peer->is_active = false;
        memset(peer->display_name, 0, sizeof(peer->display_name));
    }
}

void print_peer_list(PeerList* list) {
    printf("=== peers ===\n");
    printf("count: %ld\n", list->count);

    for (size_t i = 0; i < list->capacity; ++i) {
        PeerInfo* peer = &list->peers[i];

        if (!peer->is_active) {
            continue;
        }

        printf("\t(%ld) - %s\n", peer->id, strlen(peer->display_name) > 0 ? peer->display_name : "NO NAME");
    }
}