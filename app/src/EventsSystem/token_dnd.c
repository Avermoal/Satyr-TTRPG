#include "EventsSystem/token_dnd.h"

#include <string.h>

#include "Tokens/tokens.h"

GdkContentProvider* on_token_drag_prepare(GtkDragSource* source, double x, double y, gpointer userdata)
{
  tokensdata* tdata = (tokensdata*)userdata;
  int lenstr = 16;
  char str[lenstr];
  snprintf(str, lenstr, "%d-%d", tdata->type, tdata->id);
  GBytes* bytes = g_bytes_new(str, strlen(str));
  GdkContentProvider* provider = gdk_content_provider_new_for_bytes("text/plain", bytes);
  g_bytes_unref(bytes);
  gtk_drag_source_set_actions(source, GDK_ACTION_COPY);
  return provider;
}

void on_token_drag_begin(GtkDragSource* source, GdkDrag* drag, GtkWidget* img)
{
  GdkPaintable* paintable = GDK_PAINTABLE(gtk_widget_paintable_new(img));
  gtk_drag_source_set_icon(source, paintable, 0, 0);
  g_object_unref(paintable);
}
