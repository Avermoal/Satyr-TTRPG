#ifndef EVENTSSYSTEM_FILE_DND_H
#define EVENTSSYSTEM_FILE_DND_H

#include <gtk/gtk.h>

gboolean on_toks_drop(GtkDropTargetAsync* target, GdkDrop* drop,
                       double x, double y, gpointer userdata);

gboolean on_maps_drop(GtkDropTargetAsync* target, GdkDrop* drop,
                       double x, double y, gpointer userdata);

#endif/*EVENTSSYSTEM_FILE_DND_H*/
