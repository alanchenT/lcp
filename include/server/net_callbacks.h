#ifndef LCP_SERVER_NET_CALLBACKS_H
#define LCP_SERVER_NET_CALLBACKS_H

typedef struct ClientContext ClientContext;

bool handle_chat_message(ClientContext* ctx, void* packet);

bool handle_set_display_name(ClientContext* ctx, void* packet);

bool handle_client_connect(ClientContext* ctx);

bool handle_client_disconnect(ClientContext* ctx);

#endif