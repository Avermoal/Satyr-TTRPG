#ifndef MAP_TEXTURE_ATLAS_H
#define MAP_TEXTURE_ATLAS_H

#include <stdint.h>

struct layer;

typedef struct{
  int32_t x, y, w, h;
}rect;

typedef struct{
  const char* path;
  int32_t w, h;
  int32_t originalindex;
}img_info;

typedef struct{
  int32_t w, h;
  int32_t cursorx, cursory;
  int32_t shelf_h;
  int32_t used_w, used_h;
  int32_t padding;
  int32_t maxsize;
}shelfcontext;

void get_path_to_atlas(const char* textures_path, const char* atlas_dir_path, int32_t lrnum, struct layer* l, char** atlas_path);

#endif/*MAP_TEXTURE_ATLAS_H*/
