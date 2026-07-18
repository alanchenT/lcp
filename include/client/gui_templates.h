#pragma once

#include <gtk/gtk.h>

#define LCP_GUI_TYPE_CHAT_MESSAGE (lcp_chat_message_get_type())
G_DECLARE_FINAL_TYPE(LcpChatMessage, lcp_chat_message, LCP, CHAT_MESSAGE, GtkListBoxRow)

GtkWidget* lcp_chat_message_new(const char* sender, const char* content);