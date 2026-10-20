#ifndef GAMESTATE_GAME_STATE_H
#define GAMESTATE_GAME_STATE_H

#include <stdint.h>

#include "Map/camera.h"

struct map;
struct layer;

struct gamescene{
  struct camera cam;
  int32_t lrnum;
  int32_t opos;
  struct layer* l;
};

struct gamestate{
  char* gamename;
  struct map* m;
  struct gamescene* gscns;
  int32_t snum;
};

void save_game_state(struct gamestate* gstate);

struct gamestate* get_game_state(const char* path);

struct gamescene* get_game_scene(const char* path, int32_t snum);

#endif/*GAMESTATE_GAME_STATE_H*/
