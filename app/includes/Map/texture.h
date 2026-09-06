#ifndef MAP_TEXTURE_H
#define MAP_TEXTURE_H

#include <stdint.h>

struct texture{
  uint32_t id;
  int32_t width, height;
};

uint32_t load_png(const char* path, int32_t* width, int32_t* height);

void bindtexture(uint32_t id);
void unbindtexture(void);

void deletetexture(struct texture* tex);

#endif/*MAP_TEXTURE_H*/
