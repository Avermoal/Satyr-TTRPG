#include "GameState/game_state.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "GameState/file_initialize.h"
#include "Map/map.h"
#include "Map/map_data.h"
#include "Map/map_renderer.h"
#include "Map/texture.h"
#include "Map/camera.h"
#include "Tokens/tokens.h"

static char* get_game_path(const char* name);
static char* get_basename(char* path);
/*If scene element not save just return false*/
static bool save_game_scene(struct gamescene* gscn, FILE* state);
static struct gamescene* read_game_scenes(int32_t snum, const char* gamename, FILE* state);

void save_game_state(struct gamestate* gstate)
{
  if(!gstate->gamename){
    return;
  }
  char* path = get_game_path(gstate->gamename);
  /*Open file*/
  FILE* state = fopen(path, "wb");
  free(path);
  if(!state){
    perror("Game state not save :(\n");
    return;
  }
  /*Save from end of structure*/
  if(fwrite(&gstate->mnum, sizeof(int32_t), 1, state) < 1){
    fclose(state);
    perror("Game state save interrapt on mnum :(\n");
    return;
  }
  if(fwrite(&gstate->tnum, sizeof(int32_t), 1, state) < 1){
    fclose(state);
    perror("Game state save interrapt on tnum :(\n");
    return;
  }
  if(fwrite(&gstate->current_map_id, sizeof(int32_t), 1, state) < 1){
    fclose(state);
    perror("Game state save interrapt on current_map_id :(\n");
    return;
  }
  if(fwrite(&gstate->snum, sizeof(int32_t), 1, state) < 1){
    fclose(state);
    perror("Game state save interrapt on snum :(\n");
    return;
  }
  /*Save maps tokens*/
  for(int32_t i = 0; i < gstate->mnum; ++i){
    if(!savetoken(&gstate->maps[i], state)){
      perror("Game state save interrapt on maps :(\n");
      fclose(state);
      return;
    }
  }
  /*Save tokens tokens*/
  for(int32_t i = 0; i < gstate->tnum; ++i){
    if(!savetoken(&gstate->tokens[i], state)){
      perror("Game state save interrapt on tokens :(\n");
      fclose(state);
      return;
    }
  }
  /*Save game scenes*/
  for(int32_t i = 0; i < gstate->snum; ++i){
    if(!save_game_scene(&gstate->gscns[i], state)){
      perror("Game state save interrapt on scenes :(\n");
      fclose(state);
      return;
    }
  }
  /*Free*/
  if(fclose(state) == EOF){
    fprintf(stderr, "ERROR OF FILE SAVE %s (%s : %d)\n", path, strerror(errno), errno);
    return;
  }
  printf("File saved! :)\n");
}

void destroy_game_state(struct gamestate* gstate)
{
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
  int32_t current_map_id = 0;
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
  if(fread(&current_map_id, sizeof(int32_t), 1, state) != 1){
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
  gstate->current_map_id = current_map_id;
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
      settoken(&tmpt[gstate->tnum], TOKEN, gstate->tnum, tokpath, NULL);
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

struct gamescene* get_game_scene(const char* path, int32_t snum)
{
  return nullptr;
}

struct layer* current_scene_add_layer(struct gamestate* gstate, uint32_t layertype, int32_t toktype)
{
  struct layer* ls = map_add_layer(gstate->m, layertype, toktype);
  if(!ls){
    return nullptr;
  }
  gstate->gscns[gstate->current_map_id - 1].l = ls;
  /*Update gscn data*/
  gstate->gscns[gstate->current_map_id - 1].lrnum = gstate->m->lrnum;
  return ls;
}

void make_current(struct gamestate* gstate, int32_t snum)
{
  if(gstate->current_map_id == snum || snum < 1 || snum >= gstate->snum){
    return;
  }
  struct map* curmap = gstate->m;
  struct map* newmap = malloc(sizeof(struct map));
  if(!newmap){
    return;
  }
  create_map_from_data(newmap, gstate->gscns[snum - 1].lrnum, gstate->gscns[snum - 1].l);
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

static char* get_basename(char* path)
{
  const char* filename = strrchr(path, '/');
  if(!filename){
    return path;
  }
  return filename + 1;
}

static bool save_game_scene(struct gamescene* gscn, FILE* state)
{
  if(fwrite(&gscn->lrnum, sizeof(int32_t), 1, state) < 1){
    return false;
  }
  /*Save layers*/
  for(int i = 0; i < gscn->lrnum; ++i){
    if(fwrite(&gscn->l[i].toktype, sizeof(int32_t), 1, state) < 1){
      return false;
    }
    if(fwrite(&gscn->l[i].gridstep, sizeof(int32_t), 1, state) < 1){
      return false;
    }
    if(fwrite(&gscn->l[i].type, sizeof(uint32_t), 1, state) < 1){
      return false;
    }
    if(fwrite(&gscn->l[i].rnum, sizeof(int32_t), 1, state) < 1){
      return false;
    }
    if(fwrite(&gscn->l[i].cap, sizeof(int32_t), 1, state) < 1){
      return false;
    }
    if(fwrite(gscn->l[i].tokid, sizeof(int32_t), gscn->l[i].rnum, state) < gscn->l[i].rnum){
      return false;
    }
    if(fwrite(gscn->l[i].rects, sizeof(struct rectangle), gscn->l[i].rnum, state) < gscn->l[i].rnum){
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
        for(int32_t k = 0; k < gscns[j].lrnum; ++j){
          layerfree(&gscns[j].l[k]);
        }
        free(gscns[j].l);
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
          for(int32_t k = 0; k < gscns[j].lrnum; ++j){
            layerfree(&gscns[j].l[k]);
          }
          free(gscns[j].l);
        }
        free(gscns);
        return nullptr;
      }
      if(fread(&gridstep, sizeof(int32_t), 1, state) < 1){
        free(ls);
        /*Free layers in other scenes*/
        for(int32_t j = s - 1; j >= 0; --j){
          for(int32_t k = 0; k < gscns[j].lrnum; ++j){
            layerfree(&gscns[j].l[k]);
          }
          free(gscns[j].l);
        }
        free(gscns);
        return nullptr;
      }
      if(fread(&type, sizeof(uint32_t), 1, state) < 1){
        free(ls);
        /*Free layers in other scenes*/
        for(int32_t j = s - 1; j >= 0; --j){
          for(int32_t k = 0; k < gscns[j].lrnum; ++j){
            layerfree(&gscns[j].l[k]);
          }
          free(gscns[j].l);
        }
        free(gscns);
        return nullptr;
      }
      if(fread(&rnum, sizeof(int32_t), 1, state) < 1){
        free(ls);
        /*Free layers in other scenes*/
        for(int32_t j = s - 1; j >= 0; --j){
          for(int32_t k = 0; k < gscns[j].lrnum; ++j){
            layerfree(&gscns[j].l[k]);
          }
          free(gscns[j].l);
        }
        free(gscns);
        return nullptr;
      }
      if(fread(&cap, sizeof(int32_t), 1, state) < 1){
        free(ls);
        /*Free layers in other scenes*/
        for(int32_t j = s - 1; j >= 0; --j){
          for(int32_t k = 0; k < gscns[j].lrnum; ++j){
            layerfree(&gscns[j].l[k]);
          }
          free(gscns[j].l);
        }
        free(gscns);
        return nullptr;
      }
      struct rectangle* rects = (struct rectangle*)calloc(cap, sizeof(struct rectangle));
      if(!rects){
        free(ls);
        /*Free layers in other scenes*/
        for(int32_t j = s - 1; j >= 0; --j){
          for(int32_t k = 0; k < gscns[j].lrnum; ++j){
            layerfree(&gscns[j].l[k]);
          }
          free(gscns[j].l);
        }
        free(gscns);
        return nullptr;
      }
      if(fread(rects, sizeof(struct rectangle), rnum, state) < rnum){
        free(rects);
        free(ls);
        /*Free layers in other scenes*/
        for(int32_t j = s - 1; j >= 0; --j){
          for(int32_t k = 0; k < gscns[j].lrnum; ++j){
            layerfree(&gscns[j].l[k]);
          }
          free(gscns[j].l);
        }
        free(gscns);
        return nullptr;
      }
      int32_t* tokid = (int32_t*)calloc(cap, sizeof(int32_t));
      if(!tokid){
        free(rects);
        free(ls);
        /*Free layers in other scenes*/
        for(int32_t j = s - 1; j >= 0; --j){
          for(int32_t k = 0; k < gscns[j].lrnum; ++j){
            layerfree(&gscns[j].l[k]);
          }
          free(gscns[i].l);
        }
        free(gscns);
        return nullptr;
      }
      if(fread(tokid, sizeof(int32_t), rnum, state) < rnum){
        free(tokid);
        free(rects);
        free(ls);
        /*Free layers in other scenes*/
        for(int32_t j = s - 1; j >= 0; --j){
          for(int32_t k = 0; k < gscns[j].lrnum; ++j){
            layerfree(&gscns[j].l[k]);
          }
          free(gscns[j].l);
        }
        free(gscns);
        return nullptr;
      }
      /*Searching texture atlas*/
      char path_to_atlas[512];
      if(type == IMG){
        if(toktype == TOKEN){
          snprintf(path_to_atlas, sizeof(path_to_atlas), "saves/%s/presets/atlas/tokens_atlas.png", gamename);
        }else if(toktype == MAP){
          snprintf(path_to_atlas, sizeof(path_to_atlas), "saves/%s/presets/atlas/%d/%d.png", gamename, (snum + 1), i);
        }
      }
      /*Load texture atlas*/
      int32_t w = 0, h = 0;
      uint32_t texid = load_png(path_to_atlas, &w, &h);
      if(texid == 0 || w < 0 || h < 0){
        free(tokid);
        free(rects);
        free(ls);
        /*Free layers in other scenes*/
        for(int32_t j = s - 1; j >= 0; --j){
          for(int32_t k = 0; k < gscns[j].lrnum; ++j){
            layerfree(&gscns[j].l[k]);
          }
          free(gscns[j].l);
        }
        free(gscns);
      }
      /*Sets layer*/
      ls[i].toktype = toktype;
      ls[i].gridstep = gridstep;
      ls[i].type = type;
      ls[i].rnum = rnum;
      ls[i].cap = cap;
      ls[i].tokid = tokid;
      ls[i].rects = rects;
      ls[i].tex.id = texid;
      ls[i].tex.width = w;
      ls[i].tex.height = h;
      /*Layer is dirty*/
      ls[i].isdirty = true;
    }
    /*Sets scene*/
    gscns[s].lrnum = lrnum;
    gscns[s].l = ls;
  }
  return gscns;
}
