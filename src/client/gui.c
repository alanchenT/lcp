#include "client/gui.h"

#include <gtk/gtk.h>
#include <stdlib.h>

#include "client/gui_templates.h"
#include "client/peer_list.h"
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

static void scroll_to_bottom_idle(gpointer user_data) {
    GtkAdjustment* adj = GTK_ADJUSTMENT(user_data);

    // Smoothly set the scrollbar position to its maximum upper bound
    gtk_adjustment_set_value(adj, gtk_adjustment_get_upper(adj));
}

void append_chat_message(GuiState* gui, const char* sender, const char* content) {
    GtkWidget* new_message = lcp_chat_message_new(sender, content);
    gtk_list_box_append(GTK_LIST_BOX(gui->message_list), new_message);

    GtkAdjustment* adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(gui->messages_scrolled_window));
    g_idle_add_once(scroll_to_bottom_idle, adjustment);
}

void update_peer_list(GuiState* gui, const PeerList* list) {
    GtkWidget* parent_container = GTK_WIDGET(gui->peer_list);

    for (GtkWidget* child = gtk_widget_get_first_child(parent_container); child != NULL;
         child = gtk_widget_get_next_sibling(child)) {}

    for (size_t i = 0; i < list->capacity; ++i) {
        PeerInfo* peer = &list->peers[i];
    }
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
    gui->messages_scrolled_window = gtk_builder_get_object(builder, "messages_scrolled_window");
    gui->message_list = gtk_builder_get_object(builder, "message_list");
    gui->peer_list = gtk_builder_get_object(builder, "peer_list");
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