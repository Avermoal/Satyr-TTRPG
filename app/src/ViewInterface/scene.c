#include "ViewInterface/scene.h"

#include <stdlib.h>

#include "UI/UI.h"
#include "Map/map.h"

void createscene(struct scene* scn, GtkWindow* win)
{
  /*Memory allocation*/
  scn->ui = (struct UI*)calloc(1, sizeof(struct UI));
  scn->map = (struct map*)calloc(1, sizeof(struct map));
  /*Set map as nullptr*/
  scn->map = nullptr;
  /*Set map as userdata*/
  g_object_set_data(G_OBJECT(win), "map", scn->map);
  /*Create UIs*/
  createUI(scn->ui, win);
}

void destroyscene(struct scene* scn)
{
  destroyUI(scn->ui);
  scn->map = nullptr;
  free(scn->ui);
  free(scn->map);
  free(scn);
}

void updateUI(struct UI* ui)
{

}

