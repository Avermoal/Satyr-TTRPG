#include "DisplaySystem/display_system.h"

#include <stdlib.h>
#include <string.h>

#include <epoxy/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "GameState/game_state.h"
#include "Map/map_data.h"
#include "GameState/file_initialize.h"
#include "EventsSystem/event_system.h"
#include "Tokens/tokens.h"
#include "EventsSystem/token_dnd.h"
#include "UI/DMs_UI.h"
#include "UI/universal_callbacks.h"

void onrealize(GtkGLArea* area, gpointer userdata)
{
  /*Make area current*/
  gtk_gl_area_make_current(area);
  if(gtk_gl_area_get_error(area) != nullptr){
    return;
  }
  /*Set max texture atlas*/
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  int max_atlas_size = 0;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_atlas_size);
  g_object_set_data(G_OBJECT(userdata), "max_atlas_size", GINT_TO_POINTER(max_atlas_size));
  /*Set flip*/
  stbi_set_flip_vertically_on_load(false);
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
  int ISDM = 1;
  if(ISDM){
    struct DMs_UI* dms_ui = (struct DMs_UI*)g_object_get_data(G_OBJECT(userdata), "dms_ui_data");
    gtk_widget_add_controller(GTK_WIDGET(dms_ui->overlay), keycontroller);
  }
    /*Create or load game state and map*/
  int ISNEW = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(userdata), "ISNEW"));
  if(ISNEW){
    /*Allocation*/
    struct gamestate* gstate = (struct gamestate*)calloc(1, sizeof(struct gamestate));
    init_game_state(gstate);
    /*Create game satate*/
    GtkEntryBuffer* entrybuffer = GTK_ENTRY_BUFFER(g_object_get_data(G_OBJECT(userdata), "campaign_name_buffer"));
    if(!create_default_game_state(gstate, gtk_entry_buffer_get_text(entrybuffer))){
      return;
    }
    /*Campaign dir init*/
    campaign_dir_init(gstate->gamename);
    /*Set data*/
    g_object_set_data(G_OBJECT(userdata), "gstate", gstate);
  }else{
    /*Game path*/
    GtkEntryBuffer* entrybuffer = GTK_ENTRY_BUFFER(g_object_get_data(G_OBJECT(userdata), "campaign_name_buffer"));
    const char* basename = gtk_entry_buffer_get_text(entrybuffer);
    char gamepath[512];
    snprintf(gamepath, sizeof(gamepath), "saves/%s/%s", basename, basename);
    struct gamestate* gstate = get_game_state(gamepath);
    if(!gstate){
      return;
    }
    init_default_state(gstate);
    /*Get layers types, get current scene textures paths*/
    int32_t total = gstate->gscns[gstate->current_scene_id].lrnum;
    uint32_t types[total];
    const char* textures_paths[total];
    char paths_buffer[total][512];
    for(int32_t i = 0; i < total; ++i){
      types[i] = gstate->gscns[gstate->current_scene_id].ls[i].type;
      if(types[i] == GRID){
        paths_buffer[i][0] = '\0';
      }else if(gstate->gscns[gstate->current_scene_id].ls[i].toktype == MAP){
        snprintf(paths_buffer[i], sizeof(paths_buffer[i]), "saves/%s/presets/atlas/%d/%d.png",
                 gstate->gamename, gstate->current_scene_id, i);
      }else{
        snprintf(paths_buffer[i], sizeof(paths_buffer[i]), "saves/%s/presets/atlas/tokens_atlas.png",
                 gstate->gamename);
      }
      textures_paths[i] = paths_buffer[i];
    }
    /*Create game render data*/
    if(!create_game_render_data(gstate, types, textures_paths)){
      return;
    }
    /*Set data*/
    g_object_set_data(G_OBJECT(userdata), "gstate", gstate);
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
  /*Save game*/
  save_game_state(gstate);
  /*Free*/
  destroy_game_state(gstate);
  free(gstate);
  gstate = nullptr;
  g_object_set_data(G_OBJECT(userdata), "gstate", nullptr);
  g_object_set_data(G_OBJECT(userdata), "max_atlas_size", nullptr);
}

gboolean onrender(GtkGLArea* area, GdkGLContext* context, gpointer userdata)
{
  /*Make area current*/
  gtk_gl_area_make_current(area);
  if(gtk_gl_area_get_error(area) != NULL){
    return FALSE;
  }
  struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(userdata), "gstate");
  if(!gstate){
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
  /*Render map*/
  glDepthMask(GL_FALSE);
  renderscene(gstate, (float)width, (float)height);
  glDepthMask(GL_TRUE);
  return TRUE;
}

gboolean ontick(GtkWidget* widget, GdkFrameClock* clock, gpointer userdata)
{
  /*Get map pointer*/
  struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(userdata), "gstate");
  if(!gstate){
    return FALSE;
  }
  /*Get time*/
  gint64 frametime = gdk_frame_clock_get_frame_time(clock);
  double currenttime = TO_CURRENT_TIME(frametime);
  if(gstate->evdata.last_frame_time == 0.0){
    gstate->evdata.last_frame_time = currenttime;
  }
  gstate->evdata.deltatime = currenttime - gstate->evdata.last_frame_time;
  gstate->evdata.last_frame_time = currenttime;
  /*Update*/
  eventpoll(gstate);
  /*Let's DRAW*/
  gtk_widget_queue_draw(widget);
  return TRUE;
}
