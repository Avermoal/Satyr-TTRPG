#include "EventsSystem/scene_dnd.h"

GdkContentProvider* on_scene_drag_prepare(GtkDragSource* source, double x, double y, gpointer userdata)
{
  int32_t sid = (int32_t)GPOINTER_TO_INT(userdata);
  int lenstr = 16;
  char str[lenstr];
  snprintf(str, lenstr, "%d", sid);
  GBytes* bytes = g_bytes_new(str, strlen(str));
  GdkContentProvider* provider = gdk_content_provider_new_for_bytes("text/plain", bytes);
  g_bytes_unref(bytes);
  gtk_drag_source_set_actions(source, GDK_ACTION_COPY);
  return provider;
}

void on_scene_drag_begin(GtkDragSource* source, GdkDrag* drag, GtkWidget* il)//image/label
{
  GdkPaintable* paintable = GDK_PAINTABLE(gtk_widget_paintable_new(il));
  gtk_drag_source_set_icon(source, paintable, 0, 0);
  g_object_unref(paintable);
}
