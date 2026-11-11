#ifndef EVENTSYSTEM_EVENT_SYSTEM_H
#define EVENTSYSTEM_EVENT_SYSTEM_H

#include <gtk/gtk.h>

struct gamestate;

gboolean on_key_pressed(GtkEventControllerKey* controller, guint keyval,
                        guint keycode, GdkModifierType state, gpointer userdata);

gboolean on_key_released(GtkEventControllerKey* controller, guint keyval,
                        guint keycode, GdkModifierType state, gpointer userdata);

gboolean onscroll(GtkEventControllerScroll* scroll, double dx, double dy, gpointer userdata);

void onmotion(GtkEventControllerMotion* motion, double x, double y, gpointer userdata);

void on_drag_begin(GtkGestureDrag* gesture, double x, double y, gpointer userdata);

void on_drag_end(GtkGestureDrag* gesture, double x, double y, gpointer userdata);

void eventpoll(struct gamestate* gstate);

#endif/*EVENTSYSTEM_EVENT_SYSTEM_H*/
