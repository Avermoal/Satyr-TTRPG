#include "GameState/game_state.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "Map/map.h"
#include "Map/map_data.h"
#include "Map/map_renderer.h"
#include "Map/camera.h"

static char* get_game_path(const char* name);
static void save_game_scene(struct gamescene* gscn, FILE* state);

void save_game_state(struct gamestate* gstate)
{
  char* path = get_game_path(gstate->gamename);
  FILE* state = fopen(path, "wb");
  fwrite(&gstate->m->cam, sizeof(struct camera), 1, state);
  fwrite(&gstate->m->lrnum, sizeof(int32_t), 1, state);
  fwrite(&gstate->m->opos, sizeof(int32_t), 1, state);
  for(int i = 0; i < gstate->m->lrnum; ++i){
    fwrite(&gstate->m->l[i].rnum, sizeof(int32_t), 1, state);
    fwrite(&gstate->m->l[i].type, sizeof(uint32_t), 1, state);
    fwrite(&gstate->m->l[i].gridstep, sizeof(int32_t), 1, state);
    fwrite(gstate->m->l[i].rects, sizeof(struct rectangle), gstate->m->l[i].rnum, state);
  }
  for(int i = 0; i < gstate->snum; ++i){
    save_game_scene(&gstate->gscns[i], state);
  }
  free(path);
  fclose(state);
}

struct gamestate* get_game_state(const char* path)
{

}

static char* get_game_path(const char* name)
{
  const char* saves = "saves";
  int size = strlen(saves) + 1 + strlen(name) + 1 + strlen(name);
  char* path = calloc(size + 1, sizeof(char));
  snprintf(path, size + 1, "%s/%s/%s", saves, name, name);
  return path;
}

static void save_game_scene(struct gamescene* gscn, FILE* state)
{
  fwrite(&gscn->cam, sizeof(struct camera), 1, state);
  fwrite(&gscn->lrnum, sizeof(int32_t), 1, state);
  fwrite(&gscn->opos, sizeof(int32_t), 1, state);
  for(int i = 0; i < gscn->lrnum; ++i){
    fwrite(&gscn->l[i].rnum, sizeof(int32_t), 1, state);
    fwrite(&gscn->l[i].type, sizeof(uint32_t), 1, state);
    fwrite(&gscn->l[i].gridstep, sizeof(int32_t), 1, state);
    fwrite(gscn->l[i].rects, sizeof(struct rectangle), gscn->l[i].rnum, state);
  }
}
