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
  float gridopacity;
  struct rectangle* rects;
  int32_t* tokid;             /*Slave*/
  int32_t toktype;
  bool isdirty;
};

static inline void layerinit(struct layer* l, uint32_t type, int32_t toktype)
{
  if(!l){
    return;
  }
  l->rnum = 0;
  l->cap = 0;
  l->type = type;
  l->toktype = toktype;
  l->rects = nullptr;
  l->tokid = nullptr;
  l->isdirty = true;
  l->gridstep = 96;
  l->gridopacity = 1.0f;
}

static inline void layerfree(struct layer* l)
{
  if(!l){
    return;
  }
  free(l->rects);
  free(l->tokid);
  l->rects = nullptr;
  l->tokid = nullptr;
  l->rnum = 0;
  l->cap = 0;
  l->isdirty = false;
}

static inline bool layer_reserve_rects(struct layer* l, int32_t need)
{
  if(!l || need < 0){
    return false;
  }
  if(need <= l->cap){
    return true;
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
  /*Deep rectangle and tokid copy*/
    /*Allocation*/
  struct rectangle* tmpr = (struct rectangle*)calloc((size_t)newcap, sizeof(struct rectangle));
  if(!tmpr){
    return false;
  }
  int32_t* tmptokid = (int32_t*)calloc((size_t)newcap, sizeof(int32_t));
  if(!tmptokid){
    free(tmpr);
    return false;
  }
    /*Copy*/
  for(int32_t i = 0; i < l->cap; ++i){
    tmpr[i] = l->rects[i];
    tmptokid[i] = l->tokid[i];
  }
  /*Set invalid token id for not registed tokens*/
  for(int32_t i = l->cap; i < newcap; ++i){
    tmptokid[i] = -1;
  }
  /*Free old pointers*/
  free(l->rects);
  free(l->tokid);
  /*Sets*/
  l->rects = tmpr;
  l->tokid = tmptokid;
  l->cap = newcap;
  l->isdirty = true;
  return true;
}

static inline void layer_resize_rects(struct layer* l, int32_t count)
{
  if(!l || count < 0){
    return;
  }
  if(!layer_reserve_rects(l, count)){
    return;
  }
  if(count > l->rnum){
    memset(l->rects + l->rnum, 0, (size_t)(count - l->rnum)*sizeof(struct rectangle));
  }
  l->rnum = count;
}

static inline struct rectangle* layer_add_empty(struct layer* l)
{
  if(!l){
    return nullptr;
  }
  if(!layer_reserve_rects(l, l->rnum + 1)){
    return nullptr;
  }
  struct rectangle* rect = &l->rects[l->rnum];
  l->rnum++;
  memset(rect, 0, sizeof(*rect));
  return rect;
}

static inline bool layer_add_rect(struct layer* l, const struct rectangle* rect)
{
  if(!l || !rect){
    return false;
  }
  struct rectangle* dst = layer_add_empty(l);
  if(!dst){
    return false;
  }
  *dst = *rect;
  return true;
}

/*!!!??NOT THREAD SAFETY??!!!*/
static inline bool layer_remove_rt_by_id(struct layer* l, int32_t id)
{
  /*Id is 0-based*/
  if(!l || id < 0 || id >= l->rnum){
    return false;
  }
  struct rectangle* tmprs = (struct rectangle*)calloc((size_t)l->cap - 1, sizeof(struct rectangle));
  int32_t* tmptids = (int32_t*)calloc((size_t)l->cap -1, sizeof(int32_t));
  if(!tmprs || !tmptids){
    free(tmprs);
    free(tmptids);
    return false;
  }
  /*Fill to id (without)*/
  for(int32_t i = 0; i < id; ++i){
    tmprs[i] = l->rects[i];
    tmptids[i] = l->tokid[i];
  }
  /*Shift*/
  for(int32_t i = id; i < l->rnum - 1; ++i){
    tmprs[i] = l->rects[i + 1];
    tmptids[i] = l->tokid[i + 1];
  }
  /*Free*/
  free(l->rects);
  free(l->tokid);
  /*Sets*/
  l->rects = tmprs;
  l->tokid = tmptids;
  l->isdirty = true;
  l->rnum--;
  l->cap--;
  return true;
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

static inline float rect_get_w(struct rectangle* rect)
{
  return rect->vert[2].x - rect->vert[0].x;
}

static inline float rect_get_h(struct rectangle* rect)
{
  return rect->vert[1].y - rect->vert[0].y;
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
  if(l->toktype != tok->type || l->rnum == 0){
    return;
  }
  l->tokid[l->rnum - 1] = tok->id;
  rect_set_UV(&l->rects[l->rnum - 1], tok->u0, tok->v0, tok->u1, tok->v1);
  l->isdirty = true;
}

static inline void layer_move_rect(struct layer* l, int32_t rid, float x, float y)
{
  if(!l || rid < 0 || rid >= l->rnum){
    return;
  }
  struct rectangle* r = &l->rects[rid];
  rect_set_XYWH(r, x - 0.5f * rect_get_w(r), y - 0.5f * rect_get_h(r), rect_get_w(r), rect_get_h(r));
  l->isdirty = true;
}

static inline bool mouse_XY_in_layer_rects(float x, float y, struct layer* l, int32_t* tokid, int32_t* rid)
{
  for(int32_t i = 0; i < l->rnum; ++i){
    if((x >= l->rects[i].vert[0].x && x <= l->rects[i].vert[2].x)
        &&
       (y >= l->rects[i].vert[0].y && y <= l->rects[i].vert[2].y)){
      *tokid = l->tokid[i];
      *rid = i;
      return true;
    }
  }
  return false;
}

static inline bool mouse_XY_in_layer_token(float x, float y, struct layer* l, int32_t* tokid, int32_t* rid)
{
  float cx = 0.0f, cy = 0.0f, cr = 0.0f, dx = 0.0f, dy = 0.0f, distsqrt = 0.0f;
  for(int32_t i = 0; i < l->rnum; ++i){
    cx = l->rects[i].vert[0].x + 0.5 * rect_get_w(&l->rects[i]);
    cy = l->rects[i].vert[0].y + 0.5 * rect_get_h(&l->rects[i]);
    cr = 0.5f * rect_get_w(&l->rects[i]);
    dx = x - cx;
    dy = y - cy;
    distsqrt = dx * dx + dy * dy;
    if(distsqrt <= (cr * cr)){
      *tokid = l->tokid[i];
      *rid = i;
      return true;
    }
  }
  return false;
}

/*Not thread safety? Fuck...*/
static inline void layer_move_rt_to_begin(struct layer* l, int32_t rid)
{
  /*If rid == 0 then move don't need*/
  if(rid <=0 || rid >= l->rnum){
    return;
  }
  struct rectangle tmpr = l->rects[rid];
  int32_t tmptokid = l->tokid[rid];
  /*Move + Shift*/
  memmove(&l->rects[1], &l->rects[0], (size_t)rid * sizeof(struct rectangle));
  memmove(&l->tokid[1], &l->tokid[0], (size_t)rid * sizeof(int32_t));
  /*To begin*/
  l->rects[0] = tmpr;
  l->tokid[0] = tmptokid;
  /*Flag*/
  l->isdirty = true;
}

#endif/*MAP_MAP_DATA_H*/
