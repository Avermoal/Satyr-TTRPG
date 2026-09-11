#ifndef MAP_MAP_DATA_H
#define MAP_MAP_DATA_H

#include <stdint.h>

#define VERTEX_PER_RECT 4

#define GRID 0
#define IMG 1
#define OTHER 2

struct renderer;

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
  struct texture tex; /*TEXTURE ATLAS*/
  struct rectangle* rects;
};

struct view{
  struct renderer* ren;
  struct layer* l;
}

#endif/*MAP_MAP_DATA_H*/
