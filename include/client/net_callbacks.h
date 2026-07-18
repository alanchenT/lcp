#ifndef LCP_CLIENT_NET_CALLBACKS_H
#define LCP_CLIENT_NET_CALLBACKS_H

typedef struct App App;

bool handle_chat_message(App* app, void* packet);

bool handle_set_display_name(App* app, void* packet);

bool handle_client_connect(App* app, void* packet);

bool handle_client_disconnect(App* app, void* packet);

bool handle_client_list(App* app, void* packet);

#endif