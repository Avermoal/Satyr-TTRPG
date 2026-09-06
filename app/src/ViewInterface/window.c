#include "ViewInterface/window.h"

#include "ViewInterface/scene.h"
#include "GameState/game_state.h"

void createwindow(GtkApplication* app, struct window* win, struct gamestate* gstate)
{
  /*GTK window create*/
  win->win = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(win->win), "Satyr TTRPG VTT");
  gtk_window_set_default_size(GTK_WINDOW(win->win), 400, 400);
  gtk_window_maximize(GTK_WINDOW(win->win));
  //g_object_set_data(G_OBJECT(win->win), "gstate", gstate);
  /*Interface stack create and add the pages*/
  win->interfacestack = GTK_STACK(gtk_stack_new());
  if(win->interfacestack){
    g_object_set_data(G_OBJECT(win->win), "interfacestack", win->interfacestack);
  }
  gtk_window_set_child(GTK_WINDOW(win->win), GTK_WIDGET(win->interfacestack));
  /*Scene create*/
  win->scn = (struct scene*)calloc(1, sizeof(struct scene));
  createscene(win->scn, GTK_WINDOW(win->win));
}

void free_window_structure(struct window* win)
{
  destroyscene(win->scn);
  free(win);
}

void update_on_idle(struct window* win)
{
  
}

void update_on_tick(struct window* win)
{

}

