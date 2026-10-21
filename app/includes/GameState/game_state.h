#ifndef GAMESTATE_GAME_STATE_H
#define GAMESTATE_GAME_STATE_H

#include <stdint.h>

#include "Map/camera.h"

struct map;
struct layer;
struct token;

struct gamescene{
  struct camera cam;
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

void save_game_state(struct gamestate* gstate);

void destroy_game_state(struct gamestate* gstate);

struct gamestate* get_game_state(const char* path);

void game_state_register_token(struct gamestate* gstate, int32_t toktype, const char* tokpath);

struct gamescene* get_game_scene(const char* path, int32_t snum);

void make_current(struct gamestate* gstate, int32_t snum);

struct gamescene* make_game_scene_from_map(struct gamestate* gstate, struct map* m);

struct gamescene* add_clear_game_scene(struct gamestate* gstate);

#endif/*GAMESTATE_GAME_STATE_H*/
