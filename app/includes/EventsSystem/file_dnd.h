#ifndef EVENTSSYSTEM_FILE_DND_H
#define EVENTSSYSTEM_FILE_DND_H

#include <gtk/gtk.h>

gboolean on_toks_drag_enter(GtkDropTargetAsync* target, GdkDrop* drop, double x, double y, gpointer userdata);

void on_toks_drag_leave(GtkDropTargetAsync* target, gpointer userdata);

gboolean on_toks_drop(GtkDropTargetAsync* target, GdkDrop* drop,
                       double x, double y, gpointer userdata);

#endif/*EVENTSSYSTEM_FILE_DND_H*/
