#include "client/gui.h"

#include <gtk/gtk.h>
#include <stdlib.h>

#include "common.h"

void set_display_name_validity_state(GuiState* gui, bool state) {
    if (!state) {
        gtk_widget_add_css_class(gui->display_name_entry, "display-name-input-error");
    } else {
        gtk_widget_remove_css_class(gui->display_name_entry, "display-name-input-error");
    }
}

static GtkWidget* create_login_screen(GuiState* gui) {
    // Main container - vertical box with spacing
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(vbox, 25);
    gtk_widget_set_margin_end(vbox, 25);
    gtk_widget_set_margin_top(vbox, 100);
    gtk_widget_set_margin_bottom(vbox, 100);

    // Title label
    GtkWidget* title = gtk_label_new(NULL);
    gtk_widget_add_css_class(title, "login-title");
    gtk_label_set_text(GTK_LABEL(title), "Login");
    gtk_box_append(GTK_BOX(vbox), title);

    // Spacer
    gtk_box_append(GTK_BOX(vbox), gtk_label_new(""));

    // Prompt label
    GtkWidget* prompt = gtk_label_new("Display Name");
    gtk_widget_add_css_class(prompt, "display-name-prompt");
    gtk_widget_set_halign(prompt, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), prompt);

    // Text entry for display name
    gui->display_name_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(gui->display_name_entry), "Your name here...");
    gtk_entry_set_max_length(GTK_ENTRY(gui->display_name_entry), CLIENT_DISPLAY_NAME_MAX_LEN - 1); // Character limit
    gtk_widget_set_hexpand(gui->display_name_entry, TRUE);
    gtk_widget_set_size_request(gui->display_name_entry, 300, -1);
    gtk_box_append(GTK_BOX(vbox), gui->display_name_entry);

    // Character counter label
    GtkWidget* char_label = gtk_label_new(NULL);
    char counter[64];
    snprintf(counter, sizeof(counter), "Maximum %d characters", CLIENT_DISPLAY_NAME_MAX_LEN - 1);
    gtk_label_set_markup(GTK_LABEL(char_label), counter);
    gtk_widget_set_halign(char_label, GTK_ALIGN_END);
    gtk_widget_add_css_class(char_label, "dim-label");
    gtk_box_append(GTK_BOX(vbox), char_label);

    // Join button
    gui->join_button = gtk_button_new_with_label("Join");
    gtk_widget_set_size_request(gui->join_button, 200, 40);
    gtk_widget_set_halign(gui->join_button, GTK_ALIGN_CENTER);
    gtk_widget_add_css_class(gui->join_button, "suggested-action");
    gtk_box_append(GTK_BOX(vbox), gui->join_button);

    // Center the vbox in the window
    GtkWidget* center_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(center_box, TRUE);
    gtk_widget_set_vexpand(center_box, TRUE);
    gtk_widget_set_halign(center_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(center_box, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(center_box), vbox);

    return center_box;
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

    gui->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(gui->window), "LCP");
    gtk_window_set_default_size(GTK_WINDOW(gui->window), 1080, 720);
    gtk_window_set_resizable(GTK_WINDOW(gui->window), TRUE);

    GtkWidget* stack = gtk_stack_new();

    gui->login_screen = create_login_screen(gui);

    gtk_stack_add_named(GTK_STACK(stack), gui->login_screen, "login");
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "login");

    gtk_window_set_child(GTK_WINDOW(gui->window), stack);
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