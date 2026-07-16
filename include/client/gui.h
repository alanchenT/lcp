#ifndef LCP_CLIENT_GUI_H
#define LCP_CLIENT_GUI_H

#include <stdbool.h>
#include <stddef.h>

typedef struct _GObject GObject;
typedef struct _GtkApplication GtkApplication;

typedef struct GuiState {
    GtkApplication* app;

    GObject* window;

    GObject* login_screen;
    GObject* display_name_entry;
    GObject* join_button;

    GObject* chat_screen;

} GuiState;

void set_display_name_validity_state(GuiState* gui, bool state);

GuiState* alloc_gui(void);

void free_gui(GuiState* gui);

bool start_gui(GuiState* gui, void (*activate_hooks)(void*, void*), void* arg);

#endif