#ifndef MAP_MAP_H
#define MAP_MAP_H

#include <stdint.h>

#include "Map/camera.h"
#include "Map/map_data.h"

struct map{
  struct camera cam;
  int32_t vnum;
  struct view* v;
};

void createmap(struct map* m);

void destroymap(struct map* m);

void loadmap(struct map* m, const char* path);

void uploadmap(const struct map* m, const char* path);

void rendermap(const struct map* m, float ww, float wh); /*widget width, widget height*/

void updatemap(struct map* m);

#endif/*MAP_MAP_H*/
