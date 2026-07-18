#include "client/gui_templates.h"

struct _LcpChatMessage {
    GtkListBoxRow parent_instance;
    GtkWidget* sender_label;
    GtkWidget* content_label;
};

G_DEFINE_TYPE(LcpChatMessage, lcp_chat_message, GTK_TYPE_LIST_BOX_ROW)

static void lcp_chat_message_class_init(LcpChatMessageClass* class) {
    GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(class);

    gtk_widget_class_set_template_from_resource(widget_class, "/com/lcp/chat_message.ui");
    gtk_widget_class_bind_template_child(widget_class, LcpChatMessage, sender_label);
    gtk_widget_class_bind_template_child(widget_class, LcpChatMessage, content_label);
}

static void lcp_chat_message_init(LcpChatMessage* self) {
    gtk_widget_init_template(GTK_WIDGET(self));
}

GtkWidget* lcp_chat_message_new(const char* sender, const char* text) {
    LcpChatMessage* msg = g_object_new(LCP_GUI_TYPE_CHAT_MESSAGE, NULL);
    gtk_label_set_text(GTK_LABEL(msg->sender_label), sender);
    gtk_label_set_text(GTK_LABEL(msg->content_label), text);

    return GTK_WIDGET(msg);
}