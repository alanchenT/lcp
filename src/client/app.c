#include "client/app.h"

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#include "client/client.h"
#include "client/gui.h"
#include "client/net_callbacks.h"
#include "client/peer_list.h"
#include "debug.h"
#include "packet_queue.h"
#include "packets.h"

static gboolean poll_client_net_updates(gpointer user_data) {
    App* app = (App*)user_data;

    if (!app->client->is_connected) {
        return G_SOURCE_CONTINUE;
    }

    Client* client = app->client;

    while (client_has_packets(client)) {
        void* packet = client_poll_packet(client);
        if (packet == NULL) {
            break;
        }

        switch (get_packet_id(packet)) {
            case PACKET_ID(PacketClientHandshake): {
                PacketClientHandshake* handshake = packet;
                client->id = handshake->client_id;

                break;
            }
            case PACKET_ID(PacketChatMessage): {
                handle_chat_message(app, packet);
                break;
            }
            case PACKET_ID(PacketSetDisplayName): {
                handle_set_display_name(app, packet);
                break;
            }
            case PACKET_ID(PacketClientConnect): {
                handle_client_connect(app, packet);
                break;
            }
            case PACKET_ID(PacketClientDisconnect): {
                handle_client_disconnect(app, packet);
                break;
            }
            case PACKET_ID(PacketClientList): {
                handle_client_list(app, packet);
                break;
            }

            default: {
                break;
            }
        }

        free_packet(packet);
    }

    return G_SOURCE_CONTINUE;
}

static void send_message(void*, App* app) {
    GuiState* gui = app->gui;

    const char* message = gtk_editable_get_text(GTK_EDITABLE(gui->message_entry));
    if (message == NULL || g_utf8_strlen(message, -1) <= 0) {
        return;
    }

    ALLOC_PACKET(PacketChatMessage, packet);
    packet->client_id = app->client->id;
    strncpy(packet->message, message, sizeof(packet->message) - 1);
    packet->message[sizeof(packet->message) - 1] = '\0';
    client_send(app->client, packet);

    // Clear entry when done
    gtk_editable_set_text(GTK_EDITABLE(gui->message_entry), "");
}

static void on_message_entry_changed(GtkEditable* entry, App* app) {
    GuiState* gui = app->gui;

    const char* message = gtk_editable_get_text(entry);

    // Disable the send button if the message is empty
    gtk_widget_set_sensitive(GTK_WIDGET(gui->send_message_button), g_utf8_strlen(message, -1) > 0);
}

App* alloc_app(void) {
    App* app = malloc(sizeof(App));
    if (app == NULL) {
        return NULL;
    }

    app->client = alloc_client();
    if (app->client == NULL) {
        fprintf(stderr, "client: failed to alloc client\n");
        return NULL;
    }

    app->gui = alloc_gui();
    if (app->gui == NULL) {
        fprintf(stderr, "client: failed to alloc gui\n");
        return NULL;
    }

    app->peers = alloc_peer_list();

    return app;
}

bool app_connect(App* app, const char* port) {
    if (app->client->is_connected) {
        return false;
    }

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

    if (!app_connect(app, SERVER_PORT)) {
        return;
    }

    // Block till we complete the handshake
    if (!client_await_handshake(app->client)) {
        fprintf(stderr, "client: handshake failed\n");
        return;
    }

    ALLOC_PACKET(PacketSetDisplayName, packet);
    packet->client_id = app->client->id;
    strncpy(packet->display_name, display_name, sizeof(packet->display_name));
    client_send(app->client, packet);

    app->poll_timer_id = g_timeout_add(APP_FRAME_MS, poll_client_net_updates, app);

    set_visible_screen(gui, "chat");
}

static void on_window_close(void*, App* app) {
    if (app->poll_timer_id > 0) {
        g_source_remove(app->poll_timer_id);
    }

    client_disconnect(app->client);
    reset_peer_list(app->peers);
}

static void activate_gui_hooks(void*, void* arg) {
    App* app = arg;
    GuiState* gui = app->gui;

    // Login by pressing Enter or clicking the join button
    g_signal_connect(gui->display_name_entry, "activate", G_CALLBACK(on_login), app);
    g_signal_connect(gui->join_button, "clicked", G_CALLBACK(on_login), app);

    g_signal_connect(gui->window, "close-request", G_CALLBACK(on_window_close), app);

    g_signal_connect(gui->message_entry, "changed", G_CALLBACK(on_message_entry_changed), app);

    g_signal_connect(gui->send_message_button, "clicked", G_CALLBACK(send_message), app);
    g_signal_connect(gui->message_entry, "activate", G_CALLBACK(send_message), app);
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