#ifndef VIEWINTERFACE_WINDOW_H
#define VIEWINTERFACE_WINDOW_H

#include <gtk/gtk.h>

struct scene;
struct gamestate;

struct window{
  GtkWidget* win;
  GtkStack* interfacestack;
  struct scene* scn;
};

void createwindow(GtkApplication* app, struct window* win, struct gamestate* gstate);

void free_window_structure(struct window* win);

void update_on_idle(struct window* win);

void update_on_tick(struct window* win);

#endif/*VIEWINTERFACE_WINDOW_H*/
