#include "GameState/game_state.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "Map/map.h"
#include "Map/map_data.h"
#include "Map/map_renderer.h"
#include "Map/camera.h"
#include "Tokens/tokens.h"

static char* get_game_path(const char* name);
static void save_game_scene(struct gamescene* gscn, FILE* state);

void save_game_state(struct gamestate* gstate)/*!!!!!!!!!!!!!!!!TOKENS!!!!!!!!!!!!!!!*/
{
  if(!gstate->gamename){
    return;
  }
  char* path = get_game_path(gstate->gamename);
  FILE* state = fopen(path, "wb");
  fwrite(&gstate->current_map_id, sizeof(int32_t), 1, state);
  fwrite(&gstate->snum, sizeof(int32_t), 1, state);
  for(int i = 0; i < gstate->snum; ++i){
    save_game_scene(&gstate->gscns[i], state);
  }
  free(path);
  fclose(state);
}

void destroy_game_state(struct gamestate* gstate)
{
  if(!gstate->gamename){
    return;
  }
  for(int32_t i = 0; i < gstate->snum; ++i){
    for(int32_t j = 0; j < gstate->gscns[i].lrnum; ++j){
      layerfree(&gstate->gscns[i].l[j]);
    }
    free(gstate->gscns[i].l);
  }
  free(gstate->gscns);
  for(int32_t i = 0; i < gstate->tnum; ++i){
    free(gstate->tokens[i].path);
  }
  for(int32_t i = 0; i < gstate->mnum; ++i){
    free(gstate->maps[i].path);
  }
  free(gstate->tokens);
  free(gstate->maps);
  free(gstate->gamename);
  gstate->tokens = nullptr;
  gstate->maps = nullptr;
  gstate->gamename = nullptr;
}

struct gamestate* get_game_state(const char* path)
{
  return nullptr;
}

void game_state_register_token(struct gamestate* gstate, int32_t toktype, const char* tokpath)
{
  switch(toktype){
    case TOKEN:
      gstate->tokens = (struct token*)realloc(gstate->tokens, (gstate->tnum + 1) * sizeof(struct token));
      settoken(&gstate->tokens[gstate->tnum], toktype, gstate->tnum, tokpath);
      gstate->tnum++;
      break;
    case MAP:
      gstate->maps = (struct token*)realloc(gstate->maps, (gstate->mnum + 1) * sizeof(struct token));
      settoken(&gstate->maps[gstate->mnum], toktype, gstate->mnum, tokpath);
      gstate->mnum++;
      break;
    default:
      return;
  }
}

struct gamescene* get_game_scene(const char* path, int32_t snum)
{
  return nullptr;
}

struct layer* current_scene_add_layer(struct gamestate* gstate, uint32_t layertype, int32_t toktype)
{
  struct layer* ls = map_add_layer(gstate->m, layertype, toktype);
  gstate->gscns[gstate->current_map_id - 1].l = ls;
  /*Update gscn data*/
  gstate->gscns[gstate->current_map_id - 1].lrnum = gstate->m->lrnum;
  return ls;
}

void make_current(struct gamestate* gstate, int32_t snum)
{
  if(gstate->current_map_id == snum){
    return;
  }
  struct map* curmap = gstate->m;
  struct map* newmap = malloc(sizeof(struct map));
  create_map_from_data(newmap, gstate->gscns[snum].lrnum,
                       gstate->gscns[snum].opos, gstate->gscns[snum].l);
  destroy_map_renderer(curmap);
  free(curmap);
  gstate->m = newmap;
  gstate->current_map_id = snum;
}

struct gamescene* make_game_scene_from_map_once(struct gamestate* gstate, struct map* m)
{
  struct gamescene* gscn = (struct gamescene*)calloc(1, sizeof(struct gamescene));
  gscn->l = m->l;
  gscn->lrnum = m->lrnum;
  gscn->opos = m->opos;
  gstate->m = m;
  gstate->snum++;
  gstate->current_map_id = gstate->snum;
  return gscn;
}

struct gamescene* add_clear_game_scene(struct gamestate* gstate)
{
  return nullptr;
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
  fwrite(&gscn->lrnum, sizeof(int32_t), 1, state);
  fwrite(&gscn->opos, sizeof(int32_t), 1, state);
  for(int i = 0; i < gscn->lrnum; ++i){
    fwrite(&gscn->l[i].rnum, sizeof(int32_t), 1, state);
    fwrite(&gscn->l[i].type, sizeof(uint32_t), 1, state);
    fwrite(&gscn->l[i].gridstep, sizeof(int32_t), 1, state);
    fwrite(gscn->l[i].rects, sizeof(struct rectangle), gscn->l[i].rnum, state);
  }
}

