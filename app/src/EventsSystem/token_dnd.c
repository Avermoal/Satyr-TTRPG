#include "EventsSystem/token_dnd.h"

#include <string.h>

#include "Tokens/tokens.h"

GdkContentProvider* on_token_drag_prepare(GtkDragSource* source, double x, double y, gpointer userdata)
{
  struct token* tok = (struct token*)userdata;
  GBytes* bytes = g_bytes_new(tok->path, strlen(tok->path));
  GdkContentProvider* provider = gdk_content_provider_new_for_bytes("text/plain", bytes);
  g_bytes_unref(bytes);
  gtk_drag_source_set_actions(source, GDK_ACTION_COPY);
  return provider;
}

void on_token_drag_begin(GtkDragSource* source, GdkDrag* drag, GtkWidget* img)
{
  GdkPaintable* paintable = gtk_widget_paintable_new(img);
  gtk_drag_source_set_icon(source, paintable, 0, 0);
  g_object_unref(paintable);
  /*Get popover menu and close it*/
  GtkWidget* popover = gtk_widget_get_ancestor(img, GTK_TYPE_POPOVER);
  if(popover){
    gtk_popover_popdown(GTK_POPOVER(popover));
  }
}
