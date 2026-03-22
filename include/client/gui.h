#ifndef LCP_CLIENT_GUI_H
#define LCP_CLIENT_GUI_H

#include <stdbool.h>
#include <stddef.h>

typedef struct _GtkWidget GtkWidget;

typedef struct GuiState {
    GtkWidget* window;
} GuiState;

bool init_gui();

#endif