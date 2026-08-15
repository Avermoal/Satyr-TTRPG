#include "AppCore/application.h"

#include <stdlib.h>

#include "ViewInterface/window.h"
#include "GameState/game_state.h"

struct appsession{
  struct window* win;
  struct gamestate* gstate;
  /*struct eventssystem* evsys;*/
};

static void onactivate(GtkApplication* app, gpointer userdata);
static void on_window_destroy(GtkWidget* win, GtkApplication* app);
static void save_app_state(GtkApplication* app);

void startapp(void)
{
  /*Application session init*/
  struct appsession* session = calloc(1, sizeof(struct appsession));
  if(!session){
    return;
  }
  session->win = calloc(1, sizeof(struct window));
  if(!session->win){
    free(session);
    return;
  }
  session->gstate = calloc(1, sizeof(struct gamestate));
  if(!session->gstate){
    free(session->win);
    free(session);
    return;
  }
  /*GTK application create*/
  GtkApplication* app = gtk_application_new("io.github.avermoal.Satyr-TTRPG", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(onactivate), NULL);
  g_object_set_data(G_OBJECT(app), "session", session);
  /*G application run main loop*/
  g_application_run(G_APPLICATION(app), 0, NULL);
  /*Memmory free section*/
  free(session->gstate);
  free(session);
  destroyapp(app);
}

void destroyapp(GtkApplication* app)
{
  g_object_unref(app);
}

static void onactivate(GtkApplication* app, gpointer userdata)
{
  /*Set dark theme*/
  GtkSettings *settings = gtk_settings_get_default();
  g_object_set(settings, "gtk-theme-name", "Adwaita-dark", NULL);
  /*Set win data*/
  struct appsession* session = g_object_get_data(G_OBJECT(app), "session");
  createwindow(app, session->win, session->gstate);
  /*G signal window destroy connect*/
  g_signal_connect(session->win->win, "destroy", G_CALLBACK(on_window_destroy), app);
  /*GTK window present*/
  gtk_window_present(GTK_WINDOW(session->win->win));
}

static void on_window_destroy(GtkWidget* win, GtkApplication* app)
{
  struct appsession* session = g_object_get_data(G_OBJECT(app), "session");
  if(session){
    save_app_state(app);
  }
  if(session->win){
    free_window_structure(session->win);
  }
}

static void save_app_state(GtkApplication* app)
{
  struct appsession* session = g_object_get_data(G_OBJECT(app), "session");
  if(session && session->gstate){
  save_game_state(session->gstate);
  }
}
