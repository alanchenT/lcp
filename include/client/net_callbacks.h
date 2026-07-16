#ifndef LCP_CLIENT_NET_CALLBACKS_H
#define LCP_CLIENT_NET_CALLBACKS_H

typedef struct Client Client;
typedef struct PeerList PeerList;

bool handle_chat_message(Client* client, void* packet);

bool handle_set_display_name(PeerList* client, void* packet);

bool handle_client_connect(PeerList* client, void* packet);

bool handle_client_disconnect(PeerList* client, void* packet);

bool handle_client_list(PeerList* client, void* packet);

#endif