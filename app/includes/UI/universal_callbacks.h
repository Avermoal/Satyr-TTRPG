#ifndef UI_UNIVERSAL_CALLBACKS_H
#define UI_UNIVERSAL_CALLBACKS_H

#include <gtk/gtk.h>

/*ESCAPE*/
gboolean on_escape_pressed(GtkEventControllerKey* controller, guint keyval,
                       guint keycode, GdkModifierType state, gpointer userdata);

#endif/*UI_UNIVERSAL_CALLBACKS_H*/
