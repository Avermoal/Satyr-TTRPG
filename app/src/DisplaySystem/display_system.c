#include "DisplaySystem/display_system.h"

#include <stdlib.h>

#include <epoxy/gl.h>

#include "Map/map.h"
#include "GameState/game_state.h"
#include "GameState/file_initialize.h"
#include "EventsSystem/event_system.h"
#include "EventsSystem/token_dnd.h"
#include "UI/DMs_UI.h"

void onrealize(GtkGLArea* area, gpointer userdata)
{
  /*Make area current*/
  gtk_gl_area_make_current(area);
  if(gtk_gl_area_get_error(area) != NULL){
    return;
  }
  /*Prevent double realize*/
  struct gamestate* gamestate = (struct gamestate*)g_object_get_data(G_OBJECT(userdata), "gstate");
  if(gamestate){
    return;
  }
  /*Set controllers*/
  GtkEventController* keycontroller = gtk_event_controller_key_new();
  g_signal_connect(keycontroller, "key-pressed", G_CALLBACK(on_key_pressed), userdata);
  g_signal_connect(keycontroller, "key-released", G_CALLBACK(on_key_released), userdata);
  g_signal_connect(keycontroller, "key-pressed", G_CALLBACK(on_escape_pressed), userdata);
  gtk_widget_add_controller(GTK_WIDGET(userdata), keycontroller);
  /*Create or load game state and map*/
  int ISNEW = 1; // *(int*)g_object_get_data(G_OBJECT(userdata), "ISNEW_p");
  if(ISNEW){
    /*Allocation*/
    struct gamestate* gstate = (struct gamestate*)calloc(1, sizeof(struct gamestate));
    init_game_state(gstate);
    struct map* m = (struct map*)calloc(1, sizeof(struct map));
    createmap(m);
    /*Make game scene from map*/
    gstate->gscns = make_game_scene_from_map_once(gstate, m);
    /*Get game name*/
    GtkEntryBuffer* entrybuffer = g_object_get_data(G_OBJECT(userdata), "entrybuffer");
    gstate->gamename = strdup(gtk_entry_buffer_get_text(entrybuffer));
    /*Campaign dir init*/
    campaign_dir_init(gstate->gamename);
    /*Set data*/
    g_object_set_data(G_OBJECT(userdata), "gstate", gstate);
    g_object_set_data(G_OBJECT(userdata), "map", m);
  }else{

  }
}

void onunrealize(GtkGLArea* area, gpointer userdata)
{
  gtk_gl_area_make_current(area);
  if(gtk_gl_area_get_error(area) != NULL){
    return;
  }
  /*Prevent double unrealize*/
  struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(userdata), "gstate");
  if(!gstate){
    return;
  }
  /*Get map*/
  struct map* m = (struct map*)g_object_get_data(G_OBJECT(userdata), "map");
  /*Save game*/
  save_game_state(gstate);
  /*Free*/
  destroymap(m);
  free(m);
  m = nullptr;
  destroy_game_state(gstate);
  free(gstate);
  gstate = nullptr;
  g_object_set_data(G_OBJECT(userdata), "gstate", nullptr);
}

gboolean onrender(GtkGLArea* area, GdkGLContext* context, gpointer userdata)
{
  /*Make area current*/
  gtk_gl_area_make_current(area);
  if(gtk_gl_area_get_error(area) != NULL){
    return FALSE;
  }
  /*GL NEEDS*/
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  /*Alpha blending*/
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  /*Get area sizes*/
  int width = gtk_widget_get_width(GTK_WIDGET(area));
  int height = gtk_widget_get_height(GTK_WIDGET(area));
  if(width <= 0 || height <= 0){
    return FALSE;
  }
  /*Get map*/
  struct map* m = (struct map*)g_object_get_data(G_OBJECT(userdata), "map");
  /*Render map*/
  glDepthMask(GL_FALSE);
  rendermap(m, width, height);
  glDepthMask(GL_TRUE);
  return TRUE;
}

gboolean ontick(GtkWidget* widget, GdkFrameClock* clock, gpointer userdata)
{
  /*Get map pointer*/
  struct map* m = (struct map*)g_object_get_data(G_OBJECT(userdata), "map");
  if(!m){
    return FALSE;
  }
  /*Get time*/
  gint64 frametime = gdk_frame_clock_get_frame_time(clock);
  double currenttime = TO_CURRENT_TIME(frametime);
  if(m->evdata.last_frame_time == 0.0){
    m->evdata.last_frame_time = currenttime;
  }
  m->evdata.deltatime = currenttime - m->evdata.last_frame_time;
  m->evdata.last_frame_time = currenttime;
  /*Update*/
  updatemap(m);
  eventpoll(m);
  /*Let's DRAW*/
  gtk_widget_queue_draw(widget);
  return TRUE;
}
