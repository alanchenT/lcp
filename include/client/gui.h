#ifndef LCP_CLIENT_GUI_H
#define LCP_CLIENT_GUI_H

#include <stdbool.h>
#include <stddef.h>

typedef struct _GObject GObject;
typedef struct _GtkApplication GtkApplication;
typedef struct PeerList PeerList;

typedef struct GuiState {
    GtkApplication* app;

    GObject* window;
    GObject* stack;

    GObject* display_name_entry;
    GObject* join_button;

    // Chat screen
    GObject* message_entry;
    GObject* send_message_button;
    GObject* messages_scrolled_window;
    GObject* message_list;

    GObject* peer_list;

    GObject* chat_screen;

} GuiState;

void set_display_name_validity_state(GuiState* gui, bool state);

void set_visible_screen(GuiState* gui, const char* screen_name);

void append_chat_message(GuiState* gui, const char* sender, const char* content);

void update_peer_list(GuiState* gui, const PeerList* list);

GuiState* alloc_gui(void);

void free_gui(GuiState* gui);

bool start_gui(GuiState* gui, void (*activate_hooks)(void*, void*), void* arg);

#endif