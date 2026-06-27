#include "ViewInterface/scene.h"

#include <stdlib.h>

#include "UI/UI.h"

void createscene(struct scene* scn, GtkWindow* win)
{
  /*Memory allocation*/
  scn->ui = (struct UI*)calloc(1, sizeof(struct UI));
  /*Create UIs*/
  createUI(scn->ui, win);
  /*Create map*/
}

void destroyscene(struct scene* scn)
{
  destroyUI(scn->ui);
  free(scn);
}

void updateUI(struct UI* ui)
{

}

void updatemap()
{

}

