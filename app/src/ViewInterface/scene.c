#include "ViewInterface/scene.h"

#include <stdlib.h>

#include "UI/UI.h"
#include "Map/map.h"

void createscene(struct scene* scn, GtkWindow* win)
{
  /*Memory allocation*/
  scn->ui = (struct UI*)calloc(1, sizeof(struct UI));
  /*Set map as nullptr*/
  scn->map = nullptr;
  /*Set map as userdata*/
  g_object_set_data(G_OBJECT(win), "map_p", &scn->map);
  /*Create UIs*/
  createUI(scn->ui, win);
}

void destroyscene(struct scene* scn)
{
  destroyUI(scn->ui);
  free(scn->map);
  scn->map = nullptr;
  free(scn->ui);
}

void updateUI(struct UI* ui)
{

}

