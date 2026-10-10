#include "DisplaySystem/display_system.h"

#include <stdlib.h>

#include <epoxy/gl.h>

#include "Map/map.h"
#include "GameState/game_state.h"
#include "EventsSystem/event_system.h"

void onrealize(GtkGLArea* area, gpointer userdata)
{
  /*Make area current*/
  gtk_gl_area_make_current(area);
  if(gtk_gl_area_get_error(area) != NULL){
    return;
  }
  /*Set controllers*/
  GtkEventController* keycontroller = gtk_event_controller_key_new();
  g_signal_connect(keycontroller, "key-pressed", G_CALLBACK(on_key_pressed), userdata);
  g_signal_connect(keycontroller, "key-released", G_CALLBACK(on_key_released), userdata);
  gtk_widget_add_controller(GTK_WIDGET(userdata), keycontroller);
  /*Get map*/
  struct map** m_p = (struct map**)g_object_get_data(G_OBJECT(userdata), "map_p");
  /*Create map or load map*/
  *m_p = (struct map*)malloc(sizeof(struct map));
  if(!(*m_p)){
    g_print("Map memory allocation failed\n");
  }
  createmap(*m_p);
  /*Game state*/
  struct gamestate* gstate = (struct gamestate*)calloc(1, sizeof(struct gamestate));
  set_game_state(gstate, *m_p, "SOMEGAME");
  g_object_set_data(G_OBJECT(userdata), "gstate", gstate);
}

void onunrealize(GtkGLArea* area, gpointer userdata)
{
  gtk_gl_area_make_current(area);
  if(gtk_gl_area_get_error(area) != NULL){
    return;
  }
  /*Get map and state*/
  struct map** m_p = (struct map**)g_object_get_data(G_OBJECT(userdata), "map_p");
  struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(userdata), "gstate");
  /*Save game*/
  save_game_state(gstate);
  /*Free*/
  destroymap(*m_p);
  free(gstate);
}

gboolean onrender(GtkGLArea* area, GdkGLContext* context, gpointer userdata)
{
  /*Make area current*/
  gtk_gl_area_make_current(area);
  if(gtk_gl_area_get_error(area) != NULL){
    return FALSE;
  }
  /*GL NEEDS*/
  glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  /*Get area sizes*/
  int width = gtk_widget_get_width(GTK_WIDGET(area));
  int height = gtk_widget_get_height(GTK_WIDGET(area));
  if(width <= 0 || height <= 0){
    return FALSE;
  }
  /*Get map*/
  struct map** m_p = (struct map**)g_object_get_data(G_OBJECT(userdata), "map_p");
  /*Render map*/
  rendermap(*m_p, width, height);
  return TRUE;
}

gboolean ontick(GtkWidget* widget, GdkFrameClock* clock, gpointer userdata)
{
  /*Get map pointer*/
  struct map** m_p = (struct map**)g_object_get_data(G_OBJECT(userdata), "map_p");
  if(!m_p || !(*m_p)){
    return FALSE;
  }
  /*Get time*/
  gint64 frametime = gdk_frame_clock_get_frame_time(clock);
  double currenttime = TO_CURRENT_TIME(frametime);
  if((*m_p)->evdata.last_frame_time == 0.0){
    (*m_p)->evdata.last_frame_time = currenttime;
  }
  (*m_p)->evdata.deltatime = currenttime - (*m_p)->evdata.last_frame_time;
  (*m_p)->evdata.last_frame_time = currenttime;
  /*Update*/
  updatemap(*m_p);
  eventpoll(*m_p);
  /*Let's DRAW*/
  gtk_widget_queue_draw(widget);
  return TRUE;
}
