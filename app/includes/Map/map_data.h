#ifndef MAP_MAP_DATA_H
#define MAP_MAP_DATA_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "Map/texture.h"
#include "Tokens/tokens.h"

#define VERTEX_PER_RECT 4
#define INDEX_PER_RECT 6

enum{
  GRID = 0,
  IMG = 1
};

struct vertex{
  float x, y;
  float u, v;
  float r, g, b, a;
};

struct rectangle{
  struct vertex vert[VERTEX_PER_RECT];
};

struct layer{
  int32_t rnum;
  int32_t cap;
  uint32_t type;
  int32_t gridstep;
  struct texture tex;         /*TEXTURE ATLAS*/
  struct rectangle* rects;
  int32_t* tokid;             /*Slave*/
  int32_t toktype;
};

static inline void layerinit(struct layer* l, uint32_t type, int32_t toktype)
{
  if(!l){
    return;
  }
  l->rnum = 0;
  l->cap = 0;
  l->type = type;
  l->tex.id = 0;
  l->tex.width = 0;
  l->tex.height = 0;
  l->toktype = toktype;
  l->rects = nullptr;
  l->tokid = nullptr;
}

static inline void layerfree(struct layer* l)
{
  if(!l){
    return;
  }
  free(l->rects);
  free(l->tokid);
  if(l->tex.id != 0){
    deletetexture(&l->tex);
  }
  l->rects = nullptr;
  l->tokid = nullptr;
  l->rnum = 0;
  l->cap = 0;
}

static inline void layer_reserve_rects(struct layer* l, int32_t need)
{
  if(!l || need < 0 || need <= l->cap){
    return;
  }
  int32_t newcap = l->cap;
  if(newcap == 0){
    newcap = 8;
  }
  while(newcap < need){
    if(newcap > INT32_MAX / 2){
      newcap = need;
      break;
    }
    newcap *= 2;
  }
  if(newcap < need){
    newcap = need;
  }
  struct rectangle* tmpr = (struct rectangle*)realloc(l->rects, (size_t)(newcap * sizeof(*tmpr)));
  if(!tmpr){
    return;
  }
  int32_t* tmptokid = (int32_t*)realloc(l->tokid, (size_t)(newcap * sizeof(*tmptokid)));
  if(!tmptokid){
    free(tmpr);
    return;
  }
  l->rects = tmpr;
  l->tokid = tmptokid;
  /*Set not registerred tokid to -1*/
  for(int32_t i = l->rnum; i < newcap; ++i){
    l->tokid[i] = -1;
  }
  l->cap = newcap;
}

static inline void layer_resize_rects(struct layer* l, int32_t count)
{
  if(!l || count < 0){
    return;
  }
  layer_reserve_rects(l, count);
  if(count > l->rnum){
    memset(l->rects + l->rnum, 0, (size_t)(count - l->rnum)*sizeof(struct rectangle));
  }
  l->rnum = count;
}

static inline struct rectangle* layer_add_empty(struct layer* l)
{
  layer_reserve_rects(l, l->rnum + 1);
  struct rectangle* rect = &l->rects[l->rnum];
  l->rnum++;
  memset(rect, 0, sizeof(*rect));
  return rect;
}

static inline void layer_add_rect(struct layer* l, const struct rectangle* rect)
{
  if(!l || !rect){
    return;
  }
  struct rectangle* dst = layer_add_empty(l);
  if(!dst){
    return;
  }
  *dst = *rect;
}

static inline void rect_set_XYWH(struct rectangle* rect, float x, float y, float w, float h)
{
  if(!rect){
    return;
  }
  rect->vert[0].x = x;
  rect->vert[0].y = y;
  rect->vert[1].x = x;
  rect->vert[1].y = y + h;
  rect->vert[2].x = x + w;
  rect->vert[2].y = y + h;
  rect->vert[3].x = x + w;
  rect->vert[3].y = y;
}

static inline void rect_set_UV(struct rectangle* rect, float u0, float v0, float u1, float v1)
{
  if(!rect){
    return;
  }
  rect->vert[0].u = u0;
  rect->vert[0].v = v0;
  rect->vert[1].u = u0;
  rect->vert[1].v = v1;
  rect->vert[2].u = u1;
  rect->vert[2].v = v1;
  rect->vert[3].u = u1;
  rect->vert[3].v = v0;
}

static inline void rect_set_RGBA(struct rectangle* rect, float cr, float cg, float cb, float ca)
{
  if(!rect){
    return;
  }
  for(int32_t i = 0; i < VERTEX_PER_RECT; ++i){
    rect->vert[i].r = cr;
    rect->vert[i].g = cg;
    rect->vert[i].b = cb;
    rect->vert[i].a = ca;
  }
}

static inline void rectset(struct rectangle* rect, float x, float y, float w, float h,
                                                   float u0, float v0, float u1, float v1,
                                                   float cr, float cg, float cb, float ca)
{
  /*Position*/
  rect_set_XYWH(rect, x, y, w, h);
  /*UV coordinats*/
  rect_set_UV(rect, u0, v0, u1, v1);
  /*Color*/
  rect_set_RGBA(rect, cr, cg, cb ,ca);
}

static inline void layer_add_quad(struct layer* l, float x, float y, float w, float h,
                                                   float u0, float v0, float u1, float v1,
                                                   float cr, float cg, float cb, float ca)
{
  struct rectangle* rect = layer_add_empty(l);
  if(!rect){
    return;
  }
  rectset(rect, x, y, w, h, u0, v0, u1, v1, cr, cg, cb, ca);
}

static inline void layer_link_last_rect_with_token(struct layer* l, struct token* tok)
{
  if(l->toktype != tok->type){
    return;
  }
  l->tokid[l->rnum - 1] = tok->id;
  rect_set_UV(&l->rects[l->rnum - 1], tok->u0, tok->v0, tok->u1, tok->v1);
}

#endif/*MAP_MAP_DATA_H*/
