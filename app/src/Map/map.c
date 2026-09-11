#include "Map/map.h"

#include <stdlib.h>

#include "Map/map_renderer.h"

void createmap(struct map* m)
{
  memset(map, 0, sizeof(struct map));
  /*Camera*/
  map->cam.zoom = 1.0f;
  /*View*/
    /*Renderer*/
  for(int32_t i = 0; i < map->vnum; ++i){
    createrenderer(map->v->ren);
  }
}

void destroymap(struct map* m);

void loadmap(struct map* m, const char* path);

void uploadmap(const struct map* m, const char* path);

void rendermap(const struct map* m, float ww, float wh)
{
  /*Camera*/

}

void updatemap(struct map* m);

