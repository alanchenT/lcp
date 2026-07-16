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

    while (!client_has_packets(client)) {
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
                handle_chat_message(client, packet);
                break;
            }
            case PACKET_ID(PacketSetDisplayName): {
                handle_set_display_name(app->peers, packet);
                break;
            }
            case PACKET_ID(PacketClientConnect): {
                handle_client_connect(app->peers, packet);
                break;
            }
            case PACKET_ID(PacketClientDisconnect): {
                handle_client_disconnect(app->peers, packet);
                break;
            }
            case PACKET_ID(PacketClientList): {
                handle_client_list(app->peers, packet);
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

// static void app_main(App* app) {
//     while (app->client->is_connected) {
//         update(app->client);

//         ALLOC_PACKET(PacketChatMessage, chat_message);
//         chat_message->client_id = app->client->id;

//         printf("message: ");
//         if (fgets(chat_message->message, sizeof(chat_message->message), stdin) == NULL) {
//             free_packet(chat_message);
//             break;
//         }

//         chat_message->message[strcspn(chat_message->message, "\n")] = '\0';

//         client_send(app->client, chat_message);
//     }
// }

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

    printf("name: %s\n", display_name);

    if (!app_connect(app, SERVER_PORT)) {
        return;
    }

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