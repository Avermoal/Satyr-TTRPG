#ifndef EVENTSSYSTEMT_TOKEN_DND_H
#define EVENTSSYSTEMT_TOKEN_DND_H

#include <gtk/gtk.h>

GdkContentProvider* on_token_drag_prepare(GtkDragSource* source, double x, double y, gpointer userdata);

void on_token_drag_begin(GtkDragSource* source, GdkDrag* drag, GtkWidget* box);

#endif/*EVENTSSYSTEMT_TOKEN_DND_H*/
