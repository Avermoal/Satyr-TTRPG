#ifndef EVENTSSYSTEM_FILE_DND_H
#define EVENTSSYSTEM_FILE_DND_H

#include <gtk/gtk.h>

gboolean on_files_drop(GtkDropTarget* target, const GValue* value,
                       double x, double y, gpointer userdata);

#endif/*EVENTSSYSTEM_FILE_DND_H*/
