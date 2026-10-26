#include "ViewInterface/window.h"

#include <stdlib.h>

#include "UI/UI.h"

void createwindow(GtkApplication* app, struct window* win)
{
  /*GTK window create*/
  win->win = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(win->win), "Satyr TTRPG VTT");
  gtk_window_set_default_size(GTK_WINDOW(win->win), 400, 400);
  gtk_window_maximize(GTK_WINDOW(win->win));
  /*Interface stack create and add the pages*/
  win->interfacestack = GTK_STACK(gtk_stack_new());
  if(win->interfacestack){
    g_object_set_data(G_OBJECT(win->win), "interfacestack", win->interfacestack);
  }
  gtk_window_set_child(GTK_WINDOW(win->win), GTK_WIDGET(win->interfacestack));
  /*Memory allocation for UI*/
  win->ui = (struct UI*)calloc(1, sizeof(struct UI));
  /*Create UIs*/
  createUI(win->ui, GTK_WINDOW(win->win));
}

void free_window_structure(struct window* win)
{
  free(win->ui);
  win->ui = nullptr;
}

void update_on_idle(struct window* win)
{
  
}

void update_on_tick(struct window* win)
{

}

