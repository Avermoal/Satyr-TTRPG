#include "AppCore/application.h"

#include <stdlib.h>

#include <pango/pangocairo.h>

#include "ViewInterface/window.h"
#include "GameState/game_state.h"

#define PATH_TO_CUSTOM_CSS_THEME_MENU "res/themes/satyr.css"
#define CINZEL_PATH "res/fonts/CinzelDecorative-Bold.ttf"
#define MANROPE_PATH "res/fonts/Manrope-Medium.ttf"

struct appsession{
  struct window* win;
  struct gamestate* gstate;
  /*struct eventssystem* evsys;*/
};

static void onactivate(GtkApplication* app, gpointer userdata);
static void on_window_destroy(GtkWidget* win, GtkApplication* app);
static void save_app_state(GtkApplication* app);

static void load_custom_theme(const char* path);
static void load_custom_fonts(const char* path);

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
  /*Add custom fonts*/
  load_custom_fonts(CINZEL_PATH);
  load_custom_fonts(MANROPE_PATH);
  /*Set custom css theme*/
  load_custom_theme(PATH_TO_CUSTOM_CSS_THEME_MENU);
  /*Set win data*/
  struct appsession* session = g_object_get_data(G_OBJECT(app), "session");
  /*Add CSS class to main window*/
  createwindow(app, session->win, session->gstate);
  gtk_widget_add_css_class(GTK_WIDGET(session->win->win), "satyr-window");
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
    session->win = nullptr;
  }
}

static void save_app_state(GtkApplication* app)
{
  struct appsession* session = g_object_get_data(G_OBJECT(app), "session");
  if(session && session->gstate){
    save_game_state(session->gstate);
  }
}

static void load_custom_theme(const char* path)
{
  GtkCssProvider* provider = gtk_css_provider_new();

  gtk_css_provider_load_from_path(provider, path);
  gtk_style_context_add_provider_for_display(
    gdk_display_get_default(),
    GTK_STYLE_PROVIDER(provider),
    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
  );
  /*Unref*/
  g_object_unref(provider);
}

static void load_custom_fonts(const char* path)
{
  PangoFontMap* fontmap = pango_cairo_font_map_get_default();
  GError* err = nullptr;

  gboolean success = pango_font_map_add_font_file(fontmap, path, &err);
  if(!success){
    g_printerr("APP: Loading font error: %s: %s", path, err->message);
    g_error_free(err);
  }
}
