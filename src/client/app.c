#include "client/app.h"

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#include "client/client.h"
#include "client/gui.h"
#include "debug.h"

App* alloc_app(void) {
    App* app = malloc(sizeof(App));
    if (app == NULL) {
        return NULL;
    }

    app->client = alloc_client();
    if (app->client == NULL) {
        return NULL;
    }

    app->gui = alloc_gui();
    if (app->gui == NULL) {
        return NULL;
    }

    return app;
}

bool app_connect(App* app, const char* port) {
    if (!client_connect(app->client, port)) {
        fprintf(stderr, "client: failed to connect to server\n");
        return false;
    }

    display_peer_info(app->client->socket_fd);

    return true;
}

static void on_login(void*, App* app) {
    GuiState* gui = app->gui;

    // Get the entered name
    const char* display_name = gtk_editable_get_text(GTK_EDITABLE(gui->display_name_entry));

    if (display_name == NULL || strlen(display_name) < CLIENT_DISPLAY_NAME_MIN_LEN) {
        set_display_name_validity_state(gui, false);
        return;
    }

    printf("name: %s\n", display_name);
}

static void activate_gui_hooks(void*, void* arg) {
    App* app = arg;
    GuiState* gui = app->gui;

    // Login by pressing Enter or clicking the join button
    g_signal_connect(gui->display_name_entry, "activate", G_CALLBACK(on_login), app);
    g_signal_connect(gui->join_button, "clicked", G_CALLBACK(on_login), app);
}

bool app_start(App* app) {
    start_gui(app->gui, activate_gui_hooks, app);

    return true;
}

void app_cleanup(App* app) {
    free_gui(app->gui);
    free_client(app->client);

    free(app);
}