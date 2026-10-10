#ifndef EVENTSYSTEM_EVENT_SYSTEM_H
#define EVENTSYSTEM_EVENT_SYSTEM_H

#include <gtk/gtk.h>

struct map;

gboolean on_key_pressed(GtkEventControllerKey* controller, guint keyval,
                        guint keycode, GdkModifierType state, gpointer userdata);

gboolean on_key_released(GtkEventControllerKey* controller, guint keyval,
                        guint keycode, GdkModifierType state, gpointer userdata);

void eventpoll(struct map* m);

#endif/*EVENTSYSTEM_EVENT_SYSTEM_H*/
