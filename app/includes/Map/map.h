#ifndef MAP_MAP_H
#define MAP_MAP_H

#include <stdint.h>

#include "Map/camera.h"
#include "Map/texture.h"
#include "Map/map_data.h"
#include "EventsSystem/event_data.h"

struct renderer;

struct map{
  struct camera cam;
  int32_t lrnum;
  int32_t opos;               /*opos - other layers position*/
  struct eventdata evdata;
  struct layer* l;            /*owner: game scene*/
  struct renderer* ren;       /*Slave*/
};

void createmap(struct map* m);

void destroymap(struct map* m);

void destroy_map_renderer(struct map* m);

void loadmap(struct map* m, const char* path);

void uploadmap(const struct map* m, const char* path);

void rendermap(struct map* m, float ww, float wh); /*widget width, widget height*/

void updatemap(struct map* m);

void create_map_from_data(struct map* m, struct camera cam, int32_t lrnum, int32_t opos, struct layer* l);

struct layer* map_add_layer(struct map* m, uint32_t type, int32_t toktype);

void layer_update_texture_atlas(struct layer* l, const char* atlas_path);

#endif/*MAP_MAP_H*/
