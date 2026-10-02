#include "Map/map.h"

#include <stdlib.h>
#include <string.h>

#include "Map/map_renderer.h"
#include "Map/texture_atlas.h"

static void addgrid(struct layer* l);

void createmap(struct map* m)
{
  memset(m, 0, sizeof(struct map));
  /*Camera*/
  m->cam.zoom = 1.0f;
  /*View*/
  m->lrnum = 0;
  m->opos = -1;
    /*Grid layer*/
  map_add_layer(m, GRID);
  addgrid(m->l);
    /*Renderer grid create*/
  createrenderer(&m->ren[0], &m->l[0], GRID);
}

void destroymap(struct map* m)
{
  if(!m || !m->l || !m->ren){
    return;
  }
  for(int32_t i = 0; i < m->lrnum; ++i){
    layerfree(&m->l[i]);
    destroyrenderer(&m->ren[i]);
  }
  free(m->l);
  free(m->ren);
  m->l = NULL;
  m->ren = NULL;
  m->lrnum = 0;
  m->opos = 0;
}

void loadmap(struct map* m, const char* path)
{

}

void uploadmap(const struct map* m, const char* path)
{

}

void rendermap(const struct map* m, float ww, float wh)
{
  /*Camera*/
  float l, r, b, t;
  getviewport(&m->cam, ww, wh, &l, &r, &b, &t);
  getortho(m->cam.ortho, l, r, b, t, -1.0f, 1.0f);
  /*Render*/
  for(int32_t i = 0; i < m->lrnum; ++i){
    renderlayer(&m->ren[i], &m->l[i], m->cam.ortho);
  }
}

void updatemap(struct map* m)
{

}

struct layer* map_add_layer(struct map* m, uint32_t type)
{
  if(!m){
    return nullptr;
  }
  struct layer* ltmp = (struct layer *)realloc(m->l, (size_t)(m->lrnum + 1) * sizeof(struct layer));
  if(!ltmp){
    return nullptr;
  }
  layerinit(ltmp, type);
  layer_add_empty(ltmp);
  m->l = ltmp;
  struct renderer* rtmp = (struct renderer *)realloc(m->ren, (size_t)(m->lrnum + 1) * sizeof(struct renderer));
  if(!rtmp){
    return nullptr;
  }
  m->ren = rtmp;
  int32_t index = m->lrnum;
  m->lrnum++;
  memset(&m->ren[index], 0, sizeof(struct renderer));
  m->ren[index].u_proj_loc = -1;
  m->ren[index].u_tex_loc = -1;
  return &m->l[index];
}

static void addgrid(struct layer* l)
{
  if(!l){
    return;
  }
  /*Make grid atlas*/
  char* grid_atlas_path = nullptr;
  get_path_to_atlas("res/grid/", "saves/game1/presets/atlas/game_scene_1", 0, l, &grid_atlas_path);
  /*Load atlas texture*/
  if(!grid_atlas_path){
    return;
  }
  l->tex.id = load_png(grid_atlas_path, &l->tex.width, &l->tex.height);
  free(grid_atlas_path);
  /*MAYBE NEED TO ADD POS SET TO GRID QUAD*/
}
