#include "AppCore/application.h"

#include <stdlib.h>

#include <pango/pangocairo.h>

#include "ViewInterface/window.h"
#include "GameState/game_state.h"

#define PATH_TO_CUSTOM_CSS_THEME_MENU "res/themes/satyr.css"
#define CINZEL_PATH "res/fonts/CinzelDecorative-Bold.ttf"
#define MANROPE_PATH "res/fonts/Manrope-Medium.ttf"

static void onactivate(GtkApplication* app, gpointer userdata);
static void on_window_destroy(GtkWidget* win, GtkApplication* app);

static void load_custom_theme(const char* path);
static void load_custom_fonts(GtkApplication* app, const char* path);

void startapp(void)
{
  /*Application window init*/
  struct window* win = calloc(1, sizeof(struct window));
  if(!win){
    return;
  }
  /*GTK application create*/
  GtkApplication* app = gtk_application_new("io.github.avermoal.Satyr-TTRPG", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(onactivate), nullptr);
  g_object_set_data(G_OBJECT(app), "win_p", &win);
  /*G application run main loop*/
  g_application_run(G_APPLICATION(app), 0, NULL);
  /*Memmory free section*/
  free(win);
  destroyapp(app);
}

void destroyapp(GtkApplication* app)
{
  g_object_unref(app);
}

static void onactivate(GtkApplication* app, gpointer userdata)
{
  /*Add custom fonts*/
  load_custom_fonts(app, CINZEL_PATH);
  load_custom_fonts(app, MANROPE_PATH);
  /*Set custom css theme*/
  load_custom_theme(PATH_TO_CUSTOM_CSS_THEME_MENU);
  /*Set win data*/
  struct window** win_p = g_object_get_data(G_OBJECT(app), "win_p");
  /*Add CSS class to main window*/
  createwindow(app, *win_p);
  gtk_widget_add_css_class(GTK_WIDGET((*win_p)->win), "satyr-window");
  /*G signal window destroy connect*/
  g_signal_connect((*win_p)->win, "destroy", G_CALLBACK(on_window_destroy), app);
  /*GTK window present*/
  gtk_window_present(GTK_WINDOW((*win_p)->win));
}

static void on_window_destroy(GtkWidget* win, GtkApplication* app)
{
  struct window** win_p = g_object_get_data(G_OBJECT(app), "win_p");
  if(win_p && *win_p){
    free_window_structure(*win_p);
    free(*win_p);
    *win_p = nullptr;
  }
  /*Pango font map unref (ONLY ONCE CUZ IT'S DEFAULT FONT MAP!)*/
  g_object_unref(g_object_get_data(G_OBJECT(app), MANROPE_PATH));
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

static void load_custom_fonts(GtkApplication* app, const char* path)
{
  PangoFontMap* fontmap = pango_cairo_font_map_get_default();
  GError* err = nullptr;

  gboolean success = pango_font_map_add_font_file(fontmap, path, &err);
  if(!success){
    g_printerr("APP: Loading font error: %s: %s", path, err->message);
    g_error_free(err);
  }
  g_object_set_data(G_OBJECT(app), path, fontmap);
}
