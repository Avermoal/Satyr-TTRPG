#include "Map/map.h"

#include <stdlib.h>
#include <string.h>

#include "Map/map_renderer.h"
#include "Map/texture_atlas.h"
#include "Tokens/tokens.h"

void createmap(struct map* m)
{
  memset(m, 0, sizeof(struct map));
  /*Camera*/
  m->cam.zoom = 1.0f;
  /*View*/
  m->lrnum = 0;
  m->opos = -1;
  /*Grid layer*/
  map_add_layer(m, GRID, -1);
  m->l[0].gridstep = 64;
  /*Renderer grid create*/
  createrenderer(&m->ren[0], &m->l[0], GRID);
  /*Maps layer*/
  map_add_layer(m, IMG, MAP);
  /*Maps renderer*/
  createrenderer(&m->ren[1], &m->l[1], IMG);
}

void destroymap(struct map* m)
{
  destroy_map_renderer(m);
}

void destroy_map_renderer(struct map* m)
{
  if(!m || !m->ren){
    return;
  }
  for(int32_t i = 0; i < m->lrnum; ++i){
    destroyrenderer(&m->ren[i]);
  }
  free(m->ren);
  m->ren = nullptr;
}

void loadmap(struct map* m, const char* path)
{

}

void uploadmap(const struct map* m, const char* path)
{

}

void rendermap(struct map* m, float ww, float wh)
{
  /*Camera*/
  float l, r, b, t;
  getviewport(&m->cam, ww, wh, &l, &r, &b, &t);
  getortho(m->cam.ortho, l, r, b, t, -10.0f, 10.0f);
  /*Render*/
  for(int32_t i = m->lrnum - 1; i >= 0; --i){
    renderlayer(&m->ren[i], &m->l[i], (float)i, m->cam.ortho, ww, wh, m->cam.cam_x, m->cam.cam_y);
  }
}

void updatemap(struct map* m)
{

}

void create_map_from_data(struct map* m, struct camera cam, int32_t lrnum, int32_t opos, struct layer* l)
{
  memset(m, 0, sizeof(struct map));
  /*Camera*/
  m->cam = cam;
  /*View*/
  m->lrnum = lrnum;
  m->opos = opos;
  m->l = l;
    /*Renderer*/
  for(int32_t i = 0; i < lrnum; ++i){
    createrenderer(&m->ren[i], &m->l[i], m->l[i].type);
  }
}

struct layer* map_add_layer(struct map* m, uint32_t type, int32_t toktype)
{
  if(!m){
    return nullptr;
  }
  struct layer* ltmp = (struct layer *)realloc(m->l, (size_t)(m->lrnum + 1) * sizeof(struct layer));
  if(!ltmp){
    return nullptr;
  }
  layerinit(&ltmp[m->lrnum], type, toktype);
  layer_add_empty(&ltmp[m->lrnum]);
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

void layer_update_texture_atlas(struct layer* l, const char* atlas_path)
{
  /*Delete old texture atlas*/
  deletetexture(&l->tex);
  /*Set new texture atlas*/
  int32_t w, h;
  l->tex.id = load_png(atlas_path, &w, &h);
  l->tex.width = w;
  l->tex.height = h;
}
