#ifndef EVENTSSYSTEM_SCENE_DND_H
#define EVENTSSYSTEM_SCENE_DND_H

#include <gtk/gtk.h>

#define ADDSCENE (-1)

GdkContentProvider* on_scene_drag_prepare(GtkDragSource* source, double x, double y, gpointer userdata);

void on_scene_drag_begin(GtkDragSource* source, GdkDrag* drag, GtkWidget* il);

#endif/*EVENTSSYSTEM_SCENE_DND_H*/
