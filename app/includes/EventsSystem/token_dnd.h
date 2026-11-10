#ifndef EVENTSSYSTEM_TOKEN_DND_H
#define EVENTSSYSTEM_TOKEN_DND_H

#include <gtk/gtk.h>

GdkContentProvider* on_token_drag_prepare(GtkDragSource* source, double x, double y, gpointer userdata);

void on_token_drag_begin(GtkDragSource* source, GdkDrag* drag, GtkWidget* img);

#endif/*EVENTSSYSTEM_TOKEN_DND_H*/
