#ifndef LCP_CLIENT_APP_H
#define LCP_CLIENT_APP_H

typedef struct Client Client;
typedef struct GuiState GuiState;
typedef struct PeerList PeerList;
typedef unsigned int guint;

typedef struct App {
    GuiState* gui;
    Client* client;

    PeerList* peers;

    guint poll_timer_id;
} App;

App* alloc_app(void);

bool app_connect(App* app, const char* port);

bool app_start(App* app);

void app_cleanup(App* app);

#endif