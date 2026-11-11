#include "GameState/game_state.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "GameState/file_initialize.h"
#include "Map/map_data.h"
#include "Map/map_renderer.h"
#include "Map/texture.h"
#include "Map/camera.h"
#include "Tokens/tokens.h"

static char* get_game_path(const char* name);
/*If scene element not save just return false*/
static bool save_game_scene(struct gamescene* gscn, FILE* state);
static struct gamescene* read_game_scenes(int32_t snum, const char* gamename, FILE* state);

bool create_default_game_state(struct gamestate* gstate, const char* name)
{
  if(!gstate){
    return false;
  }
  gstate->gamename = strdup(name);
  if(!gstate->gamename){
    return false;
  }
  /*Create default scene*/
  if(!append_default_game_scene(gstate)){
    free(gstate->gamename);
    return false;
  }
  gstate->current_scene_id = 0;
  /*Create render data*/
  uint32_t types[] = {GRID, IMG};
  const char* const textures_paths[] = {nullptr, nullptr};
  if(!fill_render_data(&gstate->rens, gstate->gscns->ls, gstate->gscns->lrnum,
                       types, textures_paths)){
    free(gstate->gamename);
    /*Free layers of scenes*/
    for(int32_t i = 0; i < gstate->snum; ++i){
      for(int32_t j = 0; j < gstate->gscns[i].lrnum; ++j){
        layerfree(&gstate->gscns[i].ls[j]);
      }
      free(gstate->gscns[i].ls);
    }
    free(gstate->gscns);
    return false;
  }
  return true;
}

bool append_default_game_scene(struct gamestate* gstate)
{
  if(!gstate){
    return false;
  }
  /*Allocation*/
  struct gamescene* gscns = (struct gamescene*)calloc((size_t)(gstate->snum + 1), sizeof(struct gamescene));
  if(!gscns){
    return false;
  }
  if(gstate->snum > 0 && gscns){
    memcpy(gscns, gstate->gscns, gstate->snum * sizeof(struct gamescene));
  }
  /*Free old*/
  free(gstate->gscns);
  /*Sets*/
  gstate->gscns = gscns;
  /*Add GRID layer*/
  if(!scene_add_layer(gstate, gstate->snum, GRID, -1)){
    free(gscns);
    return false;
  }
  /*Add MAP (IMG) layer*/
  if(!scene_add_layer(gstate, gstate->snum, IMG, MAP)){
    /*Free GRID layer*/
    layerfree(&gscns[gstate->snum].ls[0]);
    free(gscns[gstate->snum].ls);
    free(gscns);
    return false;
  }
  gstate->snum++;
  return true;
}

bool create_game_render_data(struct gamestate* gstate, const uint32_t* types, const char* const* textures_paths)
{
  if(!gstate){
    return false;
  }
  /*Create render data*/
  if(!fill_render_data(&gstate->rens, gstate->gscns[gstate->current_scene_id].ls, gstate->gscns[gstate->current_scene_id].lrnum,
                       types, textures_paths)){
    return false;
  }
  return true;
}

bool scene_add_layer(struct gamestate* gstate, int32_t sid, uint32_t layertype, int32_t toktype)
{
  if(!gstate || !gstate->gscns){
    return false;
  }
  struct gamescene* gscn = &gstate->gscns[sid];
  struct layer* ls = (struct layer*)calloc((size_t)gscn->lrnum + 1, sizeof(struct layer));
  if(!ls){
    return false;
  }
  if(gscn->lrnum > 0 && gscn->ls){
    memcpy(ls, gscn->ls, gscn->lrnum * sizeof(struct layer));
  }
  /*Init new layer*/
  layerinit(&ls[gscn->lrnum], layertype, toktype);
  /*Free old*/
  free(gscn->ls);
  /*Sets*/
  gscn->ls =ls;
  gscn->lrnum++;
  return true;
}

void save_game_state(struct gamestate* gstate)
{
  if(!gstate->gamename){
    return;
  }
  char* path = get_game_path(gstate->gamename);
  if(!path){
    perror("Game state not save!!!\n");
    return;
  }
  /*Open file*/
  FILE* state = fopen(path, "wb");
  if(!state){
    perror("Game state not save :(\n");
    free(path);
    return;
  }
  /*Save from end of structure*/
  if(fwrite(&gstate->mnum, sizeof(int32_t), 1, state) < 1){
    fclose(state);
    perror("Game state save interrapt on mnum :(\n");
    free(path);
    return;
  }
  if(fwrite(&gstate->tnum, sizeof(int32_t), 1, state) < 1){
    fclose(state);
    perror("Game state save interrapt on tnum :(\n");
    free(path);
    return;
  }
  if(fwrite(&gstate->current_scene_id, sizeof(int32_t), 1, state) < 1){
    fclose(state);
    perror("Game state save interrapt on current_scene_id :(\n");
    free(path);
    return;
  }
  if(fwrite(&gstate->snum, sizeof(int32_t), 1, state) < 1){
    fclose(state);
    perror("Game state save interrapt on snum :(\n");
    free(path);
    return;
  }
  /*Save maps tokens*/
  for(int32_t i = 0; i < gstate->mnum; ++i){
    if(!savetoken(&gstate->maps[i], state)){
      perror("Game state save interrapt on maps :(\n");
      free(path);
      fclose(state);
      return;
    }
  }
  /*Save tokens tokens*/
  for(int32_t i = 0; i < gstate->tnum; ++i){
    if(!savetoken(&gstate->tokens[i], state)){
      perror("Game state save interrapt on tokens :(\n");
      free(path);
      fclose(state);
      return;
    }
  }
  /*Save game scenes*/
  for(int32_t i = 0; i < gstate->snum; ++i){
    if(!save_game_scene(&gstate->gscns[i], state)){
      perror("Game state save interrapt on scenes :(\n");
      free(path);
      fclose(state);
      return;
    }
  }
  /*Free*/
  if(fclose(state) == EOF){
    fprintf(stderr, "ERROR OF FILE SAVE %s (%s : %d)\n", path, strerror(errno), errno);
    free(path);
    return;
  }
  printf("File saved! :)\n");
}

void destroy_game_state(struct gamestate* gstate)
{
  /*Free renderer data*/
  clear_render_data(&gstate->rens, gstate->gscns[gstate->current_scene_id].lrnum);
  /*Free layers of scenes*/
  for(int32_t i = 0; i < gstate->snum; ++i){
    for(int32_t j = 0; j < gstate->gscns[i].lrnum; ++j){
      layerfree(&gstate->gscns[i].ls[j]);
    }
    free(gstate->gscns[i].ls);
  }
  free(gstate->gscns);
  /*Free tokens*/
  for(int32_t i = 0; i < gstate->tnum; ++i){
    free(gstate->tokens[i].path);
  }
  for(int32_t i = 0; i < gstate->mnum; ++i){
    free(gstate->maps[i].path);
    free(gstate->maps[i].cache);
  }
  free(gstate->tokens);
  free(gstate->maps);
  free(gstate->gamename);
  gstate->tokens = nullptr;
  gstate->maps = nullptr;
  gstate->gamename = nullptr;
  gstate->snum = 0;
  gstate->tnum = 0;
  gstate->mnum = 0;
}

struct gamestate* get_game_state(const char* path)
{
  /*Game state var*/
  int32_t mnum = 0;
  int32_t tnum = 0;
  int32_t current_scene_id = 0;
  int32_t snum = 0;
  /*File open*/
  FILE* state = fopen(path, "rb");
  if(!state){
    perror("Game state not loaded!\n");
    return nullptr;
  }
  /*Read vars*/
  if(fread(&mnum, sizeof(int32_t), 1, state) != 1){
    perror("Game state not loaded!\n");
    fclose(state);
    return nullptr;
  }
  if(fread(&tnum, sizeof(int32_t), 1, state) != 1){
    perror("Game state not loaded!\n");
    fclose(state);
    return nullptr;
  }
  if(fread(&current_scene_id, sizeof(int32_t), 1, state) != 1){
    perror("Game state not loaded!\n");
    fclose(state);
    return nullptr;
  }
  if(fread(&snum, sizeof(int32_t), 1, state) != 1){
    perror("Game state not loaded!\n");
    fclose(state);
    return nullptr;
  }
  /*Read maps*/
  struct token* maps = nullptr;
  if(mnum > 0){
    maps = gettokens(mnum, state);
    if(!maps){
      perror("Game state not load maps!\n");
      fclose(state);
      return nullptr;
    }
  }
  /*Read tokens*/
  struct token* tokens = nullptr;
  if(tnum > 0){
    tokens = gettokens(tnum, state);
    if(!tokens){
      perror("Game state not load tokens!\n");
      free(maps);
      fclose(state);
      return nullptr;
    }
  }
  /*Read scenes*/
  struct gamescene* gscns = read_game_scenes(snum, get_basename(path), state);
  if(!gscns){
    perror("Game state not load scenes!\n");
    free(tokens);
    free(maps);
    fclose(state);
    return nullptr;
  }
  fclose(state);
  /*Sets*/
  struct gamestate* gstate = (struct gamestate*)calloc(1, sizeof(struct gamestate));
  if(!gstate){
    free(tokens);
    free(maps);
    return nullptr;
  }
  gstate->mnum = mnum;
  gstate->tnum = tnum;
  gstate->current_scene_id = current_scene_id;
  gstate->snum = snum;
  gstate->maps = maps;
  gstate->tokens = tokens;
  gstate->gamename = strdup(get_basename(path));
  gstate->gscns = gscns;
  return gstate;
}

void game_state_register_token(struct gamestate* gstate, int32_t toktype, const char* tokpath, const char* cache)
{
  switch(toktype){
    case TOKEN:
      struct token* tmpt = realloc(gstate->tokens, (gstate->tnum + 1) * sizeof(struct token));
      if(!tmpt){
        return;
      }
      memset(&tmpt[gstate->tnum], 0, sizeof(struct token));
      settoken(&tmpt[gstate->tnum], TOKEN, gstate->tnum, tokpath, nullptr);
      gstate->tnum++;
      gstate->tokens = tmpt;
      break;
    case MAP:
      struct token* tmpm = realloc(gstate->maps, (gstate->mnum + 1) * sizeof(struct token));
      if(!tmpm){
        return;
      }
      memset(&tmpm[gstate->mnum], 0, sizeof(struct token));
      settoken(&tmpm[gstate->mnum], MAP, gstate->mnum, tokpath, cache);
      gstate->mnum++;
      gstate->maps = tmpm;
      break;
  }
}

void make_current(struct gamestate* gstate, int32_t sid)
{
  if(!gstate || sid < 0 || sid >= gstate->snum){
    return;
  }
  if(gstate->current_scene_id == sid){
    return;
  }
  /*Get layers types, get current scene textures paths*/
  int32_t oldtotal = gstate->gscns[gstate->current_scene_id].lrnum;
  int32_t total = gstate->gscns[sid].lrnum;
  uint32_t types[total];
  const char* textures_paths[total];
  char paths_buffer[total][512];
  for(int32_t i = 0; i < total; ++i){
    types[i] = gstate->gscns[sid].ls[i].type;
    if(types[i] == GRID){
      paths_buffer[i][0] = '\0';
    }else if(gstate->gscns[sid].ls[i].toktype == MAP){
      snprintf(paths_buffer[i], sizeof(paths_buffer[i]), "saves/%s/presets/atlas/%d/%d.png",
               gstate->gamename, sid, i);
    }else{
      snprintf(paths_buffer[i], sizeof(paths_buffer[i]), "saves/%s/presets/atlas/tokens_atlas.png",
               gstate->gamename);
    }
    textures_paths[i] = paths_buffer[i];
  }
  /*Clear render data by old total lrnum*/
  clear_render_data(&gstate->rens, oldtotal);
  /*Create game render data*/
  fill_render_data(&gstate->rens, gstate->gscns[sid].ls,
                   gstate->gscns[sid].lrnum, types, textures_paths);

  gstate->current_scene_id = sid;
  /*Mark all layers of current scene like dirty*/
  for(int32_t i = 0; i < gstate->gscns[sid].lrnum; ++i){
    gstate->gscns[sid].ls[i].isdirty = true;
  }
}

void renderscene(struct gamestate* gstate, float ww, float wh)
{
  /*Camera*/
  float l, r, b, t;
  getviewport(&gstate->cam, ww, wh, &l, &r, &b, &t);
  getortho(gstate->cam.ortho, l, r, b, t, -1.0f, 1.0f);
  /*Maps*/
  int32_t total = gstate->gscns[gstate->current_scene_id].lrnum;
  struct layer* ls = gstate->gscns[gstate->current_scene_id].ls;
  for(int32_t i = total - 1; i >= 0; --i){
    if(ls[i].type != GRID && ls[i].toktype != TOKEN){
      renderlayer(&gstate->rens.ren[i], &gstate->rens.tex[i], &ls[i], (float)(-i), gstate->cam.ortho,
                  ww, wh, gstate->cam.cam_x, gstate->cam.cam_y, gstate->cam.zoom);
    }
  }
  /*Grid*/
  for(int32_t i = total - 1; i >= 0; --i){
    if(ls[i].type == GRID){
      renderlayer(&gstate->rens.ren[i], &gstate->rens.tex[i], &ls[i], (float)(-i), gstate->cam.ortho,
                  ww, wh, gstate->cam.cam_x, gstate->cam.cam_y, gstate->cam.zoom);
    }
  }
  /*Tokens*/
  for(int32_t i = 0; i < total; ++i){
    if(ls[i].toktype == TOKEN){
      renderlayer(&gstate->rens.ren[i], &gstate->rens.tex[i], &ls[i], (float)(i), gstate->cam.ortho,
                  ww, wh, gstate->cam.cam_x, gstate->cam.cam_y, gstate->cam.zoom);
    }
  }
}

char* get_basename(char* path)
{
  const char* filename = strrchr(path, '/');
  if(!filename){
    return path;
  }
  return filename + 1;
}

static char* get_game_path(const char* name)
{
  const char* saves = "saves";
  int size = strlen(saves) + 1 + strlen(name) + 1 + strlen(name);
  char* path = calloc(size + 1, sizeof(char));
  if(!path){
    return nullptr;
  }
  snprintf(path, size + 1, "%s/%s/%s", saves, name, name);
  return path;
}

static bool save_game_scene(struct gamescene* gscn, FILE* state)
{
  if(fwrite(&gscn->lrnum, sizeof(int32_t), 1, state) < 1){
    return false;
  }
  /*Save layers*/
  for(int i = 0; i < gscn->lrnum; ++i){
    if(fwrite(&gscn->ls[i].toktype, sizeof(int32_t), 1, state) < 1){
      return false;
    }
    if(fwrite(&gscn->ls[i].gridstep, sizeof(int32_t), 1, state) < 1){
      return false;
    }
    if(fwrite(&gscn->ls[i].type, sizeof(uint32_t), 1, state) < 1){
      return false;
    }
    if(fwrite(&gscn->ls[i].rnum, sizeof(int32_t), 1, state) < 1){
      return false;
    }
    if(fwrite(&gscn->ls[i].cap, sizeof(int32_t), 1, state) < 1){
      return false;
    }
    if(fwrite(gscn->ls[i].tokid, sizeof(int32_t), gscn->ls[i].rnum, state) < gscn->ls[i].rnum){
      return false;
    }
    if(fwrite(gscn->ls[i].rects, sizeof(struct rectangle), gscn->ls[i].rnum, state) < gscn->ls[i].rnum){
      return false;
    }
  }
  return true;
}

static struct gamescene* read_game_scenes(int32_t snum, const char* gamename, FILE* state)
{
  struct gamescene* gscns = (struct gamescene*)calloc(snum, sizeof(struct gamescene));
  if(!gscns){
    return nullptr;
  }
  for(int32_t s = 0; s < snum; ++s){
    /*Scene var*/
    int32_t lrnum = 0;
    if(fread(&lrnum, sizeof(int32_t), 1, state) < 1){
      /*Free layers in other scenes*/
      for(int32_t j = s - 1; j >= 0; --j){
        for(int32_t k = 0; k < gscns[j].lrnum; ++k){
          layerfree(&gscns[j].ls[k]);
        }
        free(gscns[j].ls);
      }
      free(gscns);
      return nullptr;
    }
    if(lrnum < 0 || lrnum > 64){
      /*Free layers in other scenes*/
      for(int32_t j = s - 1; j >= 0; --j){
        for(int32_t k = 0; k < gscns[j].lrnum; ++k){
          layerfree(&gscns[j].ls[k]);
        }
        free(gscns[j].ls);
      }
      free(gscns);

      return nullptr;
    }
    struct layer* ls = (struct layer*)calloc(lrnum, sizeof(struct layer));
    if(!ls){
      free(gscns);
      return nullptr;
    }
    /*Layer var*/
    int32_t toktype = 0;
    int32_t gridstep = 0;
    uint32_t type = 0;
    int32_t rnum = 0;
    int32_t cap = 0;
    /*Read layers*/
    for(int32_t i = 0; i < lrnum; ++i){
      if(fread(&toktype, sizeof(int32_t), 1, state) < 1){
        free(ls);
        /*Free layers in other scenes*/
        for(int32_t j = s - 1; j >= 0; --j){
          for(int32_t k = 0; k < gscns[j].lrnum; ++k){
            layerfree(&gscns[j].ls[k]);
          }
          free(gscns[j].ls);
        }
        free(gscns);
        return nullptr;
      }
      if(fread(&gridstep, sizeof(int32_t), 1, state) < 1){
        free(ls);
        /*Free layers in other scenes*/
        for(int32_t j = s - 1; j >= 0; --j){
          for(int32_t k = 0; k < gscns[j].lrnum; ++k){
            layerfree(&gscns[j].ls[k]);
          }
          free(gscns[j].ls);
        }
        free(gscns);
        return nullptr;
      }
      if(fread(&type, sizeof(uint32_t), 1, state) < 1){
        free(ls);
        /*Free layers in other scenes*/
        for(int32_t j = s - 1; j >= 0; --j){
          for(int32_t k = 0; k < gscns[j].lrnum; ++k){
            layerfree(&gscns[j].ls[k]);
          }
          free(gscns[j].ls);
        }
        free(gscns);
        return nullptr;
      }
      layerinit(&ls[i], type, toktype);
      if(fread(&rnum, sizeof(int32_t), 1, state) < 1){
        free(ls);
        /*Free layers in other scenes*/
        for(int32_t j = s - 1; j >= 0; --j){
          for(int32_t k = 0; k < gscns[j].lrnum; ++k){
            layerfree(&gscns[j].ls[k]);
          }
          free(gscns[j].ls);
        }
        free(gscns);
        return nullptr;
      }
      if(fread(&cap, sizeof(int32_t), 1, state) < 1){
        free(ls);
        /*Free layers in other scenes*/
        for(int32_t j = s - 1; j >= 0; --j){
          for(int32_t k = 0; k < gscns[j].lrnum; ++k){
            layerfree(&gscns[j].ls[k]);
          }
          free(gscns[j].ls);
        }
        free(gscns);
        return nullptr;
      }
      if(cap < 0 || cap > 1024){
        free(ls);
        /*Free layers in other scenes*/
        for(int32_t j = s - 1; j >= 0; --j){
          for(int32_t k = 0; k < gscns[j].lrnum; ++k){
            layerfree(&gscns[j].ls[k]);
          }
          free(gscns[j].ls);
        }
        free(gscns);
        return nullptr;
      }
      if(rnum < 0 || rnum > cap){
        free(ls);
        /*Free layers in other scenes*/
        for(int32_t j = s - 1; j >= 0; --j){
          for(int32_t k = 0; k < gscns[j].lrnum; ++k){
            layerfree(&gscns[j].ls[k]);
          }
          free(gscns[j].ls);
        }
        free(gscns);
        return nullptr; 
      }
      int32_t* tokid = (int32_t*)calloc(cap, sizeof(int32_t));
      if(!tokid){
        free(ls);
        /*Free layers in other scenes*/
        for(int32_t j = s - 1; j >= 0; --j){
          for(int32_t k = 0; k < gscns[j].lrnum; ++k){
            layerfree(&gscns[j].ls[k]);
          }
          free(gscns[j].ls);
        }
        free(gscns);
        return nullptr;
      }
      if(fread(tokid, sizeof(int32_t), rnum, state) < rnum){
        free(tokid);
        free(ls);
        /*Free layers in other scenes*/
        for(int32_t j = s - 1; j >= 0; --j){
          for(int32_t k = 0; k < gscns[j].lrnum; ++k){
            layerfree(&gscns[j].ls[k]);
          }
          free(gscns[j].ls);
        }
        free(gscns);
        return nullptr;
      }
      struct rectangle* rects = (struct rectangle*)calloc(cap, sizeof(struct rectangle));
      if(!rects){
        free(tokid);
        free(ls);
        /*Free layers in other scenes*/
        for(int32_t j = s - 1; j >= 0; --j){
          for(int32_t k = 0; k < gscns[j].lrnum; ++k){
            layerfree(&gscns[j].ls[k]);
          }
          free(gscns[j].ls);
        }
        free(gscns);
        return nullptr;
      }
      if(fread(rects, sizeof(struct rectangle), rnum, state) < rnum){
        free(rects);
        free(tokid);
        free(ls);
        /*Free layers in other scenes*/
        for(int32_t j = s - 1; j >= 0; --j){
          for(int32_t k = 0; k < gscns[j].lrnum; ++k){
            layerfree(&gscns[j].ls[k]);
          }
          free(gscns[j].ls);
        }
        free(gscns);
        return nullptr;
      }
      /*Sets layer*/
      ls[i].toktype = toktype;
      ls[i].gridstep = gridstep;
      ls[i].type = type;
      ls[i].rnum = rnum;
      ls[i].cap = cap;
      ls[i].tokid = tokid;
      ls[i].rects = rects;
      ls[i].gridopacity = 1.0f;
      /*Layer is dirty*/
      ls[i].isdirty = true;
    }
    /*Sets scene*/
    gscns[s].lrnum = lrnum;
    gscns[s].ls = ls;
  }
  return gscns;
}
