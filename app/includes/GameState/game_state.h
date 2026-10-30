#ifndef GAMESTATE_GAME_STATE_H
#define GAMESTATE_GAME_STATE_H

#include <stdint.h>

#include "Map/camera.h"

struct map;
struct layer;
struct token;

struct gamescene{
  int32_t lrnum;
  int32_t opos;
  struct layer* l;          /*Slave*/
};

struct gamestate{
  char* gamename;           /*Slave*/
  struct map* m;            /*CURRENT DISPLAY MAP. owner: gl_area*/
  struct gamescene* gscns;  /*Slave*/
  struct token* tokens;     /*Slave*/
  struct token* maps;       /*Slave*/
  int32_t snum;
  int32_t current_map_id;
  int32_t tnum;
  int32_t mnum;
};

static inline void init_game_state(struct gamestate* gstate)
{
  if(!gstate){
    return;
  }
  gstate->gamename = nullptr;
  gstate->m = nullptr;
  gstate->gscns = nullptr;
  gstate->tokens = nullptr;
  gstate->maps = nullptr;
  gstate->snum = 0;
  gstate->current_map_id = 0;
  gstate->tnum = 0;
  gstate->mnum = 0;
}

void save_game_state(struct gamestate* gstate);

void destroy_game_state(struct gamestate* gstate);

struct gamestate* get_game_state(const char* path);

void game_state_register_token(struct gamestate* gstate, int32_t toktype, const char* tokpath);

struct gamescene* get_game_scene(const char* path, int32_t snum);

struct layer* current_scene_add_layer(struct gamestate* gstate, uint32_t layertype, int32_t toktype);

void make_current(struct gamestate* gstate, int32_t snum);

struct gamescene* make_game_scene_from_map_once(struct gamestate* gstate, struct map* m);

struct gamescene* add_clear_game_scene(struct gamestate* gstate);

#endif/*GAMESTATE_GAME_STATE_H*/
