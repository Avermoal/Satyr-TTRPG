#ifndef GAMESTATE_GAME_STATE_H
#define GAMESTATE_GAME_STATE_H

#include <stdint.h>

#include "Map/map_renderer.h"
#include "Map/camera.h"
#include "EventsSystem/event_data.h"
#include "Tokens/tokens.h"

struct layer;

struct gamescene{
  int32_t lrnum;
  struct layer* ls;
};

struct gamestate{
  char* gamename;
  struct gamescene* gscns;
  struct token* tokens;
  struct token* maps;
  struct renderdata rens;
  struct camera cam;
  struct eventdata evdata;
  int32_t snum;
  int32_t current_scene_id;
  int32_t tnum;
  int32_t mnum;
};

static inline void init_game_state(struct gamestate* gstate)
{
  if(!gstate){
    return;
  }
  gstate->gamename = nullptr;
  gstate->gscns = nullptr;
  gstate->tokens = nullptr;
  gstate->maps = nullptr;
  gstate->rens.tex = nullptr;
  gstate->rens.ren = nullptr;
  gstate->cam.cam_x = 0.0f;
  gstate->cam.cam_y = 0.0f;
  gstate->cam.zoom = 1.0f;
  init_event_data(&gstate->evdata);
  gstate->snum = 0;
  gstate->current_scene_id = 0;
  gstate->tnum = 0;
  gstate->mnum = 0;
}

static inline void init_default_state(struct gamestate* gstate){
  if(!gstate){
    return;
  }
  gstate->cam.cam_x = 0.0f;
  gstate->cam.cam_y = 0.0f;
  gstate->cam.zoom = 1.0f;
  init_event_data(&gstate->evdata);
}

bool create_default_game_state(struct gamestate* gstate, const char* name);

bool append_default_game_scene(struct gamestate* gstate);

bool create_game_render_data(struct gamestate* gstate, const uint32_t* types, const char* const* textures_paths);

bool scene_add_layer(struct gamestate* gstate, int32_t sid, uint32_t layertype, int32_t toktype);

bool delete_game_scene(struct gamestate* gstate, int32_t sid);

bool delete_token_full(struct gamestate* gstate, int32_t tid, int32_t toktype);

void save_game_state(struct gamestate* gstate);

void destroy_game_state(struct gamestate* gstate);

struct gamestate* get_game_state(const char* path);

void game_state_register_token(struct gamestate* gstate, int32_t toktype, const char* tokpath, const char* cache);

void make_current(struct gamestate* gstate, int32_t sid);

void renderscene(struct gamestate* gstate, float ww, float wh);

char* get_basename(char* path);

static inline struct token* find_token_by_id(struct gamestate* gstate, int32_t id)
{
  for(int32_t i = 0; i < gstate->tnum; ++i){
    if(gstate->tokens[i].id == id){
      return &gstate->tokens[i];
    }
  }
  return nullptr;
}

static inline struct token* find_map_by_id(struct gamestate* gstate, int32_t id)
{
  for(int32_t i = 0; i < gstate->mnum; ++i){
    if(gstate->maps[i].id == id){
      return &gstate->maps[i];
    }
  }
  return nullptr;
}

#endif/*GAMESTATE_GAME_STATE_H*/
