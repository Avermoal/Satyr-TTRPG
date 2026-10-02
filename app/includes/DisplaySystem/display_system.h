#ifndef DISPLAYSYSTEM_DISPLAYSYSTEM_H
#define DISPLAYSYSTEM_DISPLAYSYSTEM_H

#include <gtk/gtk.h>

void onrealize(GtkGLArea* area, gpointer userdata);

void onunrealize(GtkGLArea* area, gpointer userdata);

gboolean onrender(GtkGLArea* area, GdkGLContext* context, gpointer userdata);

#endif/*DISPLAYSYSTEM_DISPLAYSYSTEM_H*/
