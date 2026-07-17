#include "client/gui.h"

#include <gtk/gtk.h>
#include <stdlib.h>

#include "common.h"

void set_display_name_validity_state(GuiState* gui, bool state) {
    if (!state) {
        gtk_widget_add_css_class(GTK_WIDGET(gui->display_name_entry), "error");
    } else {
        gtk_widget_remove_css_class(GTK_WIDGET(gui->display_name_entry), "error");
    }
}

void set_visible_screen(GuiState* gui, const char* screen_name) {
    gtk_stack_set_visible_child_name(GTK_STACK(gui->stack), screen_name);
}

static void setup_login_screen(GuiState* gui, GtkBuilder* builder) {
    // Text entry for display name
    gui->display_name_entry = gtk_builder_get_object(builder, "display_name_entry");
    gtk_entry_set_max_length(GTK_ENTRY(gui->display_name_entry), CLIENT_DISPLAY_NAME_MAX_LEN - 1); // Character limit

    // Character counter label
    GObject* max_char_label = gtk_builder_get_object(builder, "max_char_label");
    char counter[64];
    snprintf(counter, sizeof(counter), "Maximum %d characters", CLIENT_DISPLAY_NAME_MAX_LEN - 1);
    gtk_label_set_markup(GTK_LABEL(max_char_label), counter);

    gui->join_button = gtk_builder_get_object(builder, "join_button");
}

static void setup_chat_screen(GuiState* gui, GtkBuilder* builder) {
    gui->message_entry = gtk_builder_get_object(builder, "message_entry");
    gui->send_message_button = gtk_builder_get_object(builder, "send_message_button");
}

static void load_css(void) {
    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(provider, "/com/lcp/styles.css");
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_FALLBACK
    );
    g_object_unref(provider);
}

static void activate(GtkApplication* app, GuiState* gui) {
    load_css();

    GtkBuilder* builder = gtk_builder_new_from_resource("/com/lcp/app.ui");

    gui->window = gtk_builder_get_object(builder, "main_window");
    gtk_window_set_application(GTK_WINDOW(gui->window), app);

    // Link UI elements, apply dynamic values
    setup_login_screen(gui, builder);
    setup_chat_screen(gui, builder);

    // Present the login screen
    gui->stack = gtk_builder_get_object(builder, "main_stack");
    gtk_stack_set_visible_child_name(GTK_STACK(gui->stack), "login");

    gtk_window_present(GTK_WINDOW(gui->window));
}

GuiState* alloc_gui(void) {
    GuiState* gui = calloc(1, sizeof(GuiState));
    if (gui == NULL) {
        return NULL;
    }

    gui->app = gtk_application_new("com.lcp", G_APPLICATION_DEFAULT_FLAGS);

    return gui;
}

void free_gui(GuiState* gui) {
    g_object_unref(gui->app);
    gui->app = NULL;

    free(gui);
}

bool start_gui(GuiState* gui, void (*activate_hooks)(void*, void*), void* arg) {
    g_signal_connect(gui->app, "activate", G_CALLBACK(activate), gui);
    g_signal_connect(gui->app, "activate", G_CALLBACK(activate_hooks), arg);
    return g_application_run(G_APPLICATION(gui->app), 0, NULL);
}