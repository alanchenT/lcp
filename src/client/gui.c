#include "client/gui.h"

#include <gtk/gtk.h>

// static void display_name_box() {}

static void chat_message_prompt(GtkWidget* overlay) {
    GtkWidget* entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Type message:");
    gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);
    gtk_widget_add_css_class(entry, "read-only");

    gtk_widget_set_halign(entry, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(entry, GTK_ALIGN_START);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), entry);
}

// static void display_name_screen(GtkWidget* overlay) {}

static void activate(GtkApplication* app) {
    GtkWidget* window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "LCP");
    gtk_window_set_default_size(GTK_WINDOW(window), 1080, 720);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);

    GtkWidget* overlay = gtk_overlay_new();
    gtk_window_set_child(GTK_WINDOW(window), overlay);

    chat_message_prompt(overlay);

    gtk_window_present(GTK_WINDOW(window));
}

bool init_gui() {
    // Silence warnings
    setenv("GSK_RENDERER", "cairo", 1);

    GtkApplication* app = gtk_application_new("com.me.lcp", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);

    return true;
}