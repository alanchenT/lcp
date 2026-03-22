#ifndef LCP_CLIENT_NET_CALLBACKS_H
#define LCP_CLIENT_NET_CALLBACKS_H

typedef struct Client Client;

bool handle_chat_message(Client* client, void* packet);

bool handle_set_display_name(Client* client, void* packet);

bool handle_client_connect(Client* client, void* packet);

bool handle_client_disconnect(Client* client, void* packet);

bool handle_client_list(Client* client, void* packet);

#endif