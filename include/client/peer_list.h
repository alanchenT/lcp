#ifndef LCP_CLIENT_PEER_LIST_H
#define LCP_CLIENT_PEER_LIST_H

#include "common.h"

typedef struct PeerInfo {
    size_t id;
    char display_name[CLIENT_DISPLAY_NAME_MAX_LEN];
    bool is_active;
} PeerInfo;

typedef struct PeerList {
    PeerInfo peers[MAX_CLIENTS];
    size_t count;
    size_t capacity;
} PeerList;

PeerList* alloc_peer_list();
void free_peer_list(PeerList* list);

void add_peer(PeerList* list, size_t id);
void remove_peer(PeerList* list, size_t id);
const char* get_peer_display_name(PeerList* list, size_t id);
void set_peer_display_name(PeerList* list, size_t id, const char* display_name);
void reset_peer_list(PeerList* list);

// Debug
void print_peer_list(PeerList* list);

#endif