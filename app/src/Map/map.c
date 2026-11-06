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
  /*Grid layer*/
  map_add_layer(m, GRID, -1);
  m->l[0].gridstep = 96;
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
  getortho(m->cam.ortho, l, r, b, t, -1.0f, 1.0f);
  /*Maps*/
  for(int32_t i = m->lrnum - 1; i >= 0; --i){
    if(m->l[i].type != GRID && m->l[i].toktype != TOKEN){
      renderlayer(&m->ren[i], &m->l[i], (float)(-i), m->cam.ortho,
                  ww, wh, m->cam.cam_x, m->cam.cam_y, m->cam.zoom);
    }
  }
  /*Grid*/
  for(int32_t i = m->lrnum - 1; i >= 0; --i){
    if(m->l[i].type == GRID){
      renderlayer(&m->ren[i], &m->l[i], (float)(-i), m->cam.ortho,
                  ww, wh, m->cam.cam_x, m->cam.cam_y, m->cam.zoom);
    }
  }
  /*Tokens*/
  for(int32_t i = 0; i < m->lrnum; ++i){
    if(m->l[i].toktype == TOKEN){
      renderlayer(&m->ren[i], &m->l[i], (float)(i), m->cam.ortho,
                  ww, wh, m->cam.cam_x, m->cam.cam_y, m->cam.zoom);
    }
  }
}

void updatemap(struct map* m)
{

}

void create_map_from_data(struct map* m, int32_t lrnum, struct layer* l)
{
  memset(m, 0, sizeof(struct map));
  /*View*/
  m->cam.zoom = 1.0f;
  m->cam.cam_x = 0.0f;
  m->cam.cam_y = 0.0f;
  m->lrnum = lrnum;
  m->l = l;
  /*Renderer*/
  m->ren = (struct renderer*)calloc((size_t)lrnum, sizeof(struct renderer));
  if(!m->ren){
    return;
  }
  for(int32_t i = 0; i < lrnum; ++i){
    createrenderer(&m->ren[i], &m->l[i], m->l[i].type);
  }
}

struct layer* map_add_layer(struct map* m, uint32_t type, int32_t toktype)
{
  if(!m){
    return nullptr;
  }
  /*New lrnum*/
  size_t total = (size_t) m->lrnum + 1;
  /*Reallocation*/
  struct layer* ltmp = (struct layer*)calloc(total, sizeof(struct layer));
  /*Check*/
  if(!ltmp){
    return nullptr;
  }
  /*Reallocation*/
  struct renderer* rtmp = (struct renderer*)calloc(total, sizeof(struct renderer));
  /*Check*/
  if(!rtmp){
    free(ltmp);
    return nullptr;
  }
  /*Copy*/
  if(m->lrnum > 0){
    memcpy(ltmp, m->l, (size_t)m->lrnum * sizeof(struct layer));
    memcpy(rtmp, m->ren, (size_t)m->lrnum * sizeof(struct renderer));
  }
  /*Init*/
  layerinit(&ltmp[m->lrnum], type, toktype);
  /*New renderer invalidation*/
  struct renderer* ren = &rtmp[m->lrnum];
  ren->u_proj_loc = -1;
  ren->u_tex_loc = -1;
  ren->u_grid_step_loc = -1;
  ren->u_visible_half_x_loc = -1;
  ren->u_visible_half_y_loc = -1;
  ren->u_camera_pos_loc = -1;
  ren->u_lrnum_loc = -1;
  /*Free old*/
  free(m->l);
  free(m->ren);
  /*Sets*/
  m->l = ltmp;
  m->ren = rtmp;
  m->lrnum = total;
  /*Return new layers*/
  return m->l;
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
  l->isdirty = true;
}
