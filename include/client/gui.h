#ifndef LCP_CLIENT_GUI_H
#define LCP_CLIENT_GUI_H

#include <stdbool.h>
#include <stddef.h>

typedef struct _GtkWidget GtkWidget;
typedef struct _GtkApplication GtkApplication;

typedef struct GuiState {
    GtkApplication* app;

    GtkWidget* window;

    GtkWidget* login_screen;
    GtkWidget* display_name_entry;
    GtkWidget* join_button;

    GtkWidget* chat_screen;

} GuiState;

void set_display_name_validity_state(GuiState* gui, bool state);

GuiState* alloc_gui(void);

void free_gui(GuiState* gui);

bool start_gui(GuiState* gui, void (*activate_hooks)(void*, void*), void* arg);

#endif