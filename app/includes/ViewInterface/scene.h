#ifndef VIEWINTERFACE_SCENE_H
#define VIEWINTERFACE_SCENE_H

#include <gtk/gtk.h>

struct UI;

struct scene{
  struct UI* ui;
  //struct map* map;
};

void createscene(struct scene* scn, GtkWindow* win);

void destroyscene(struct scene* scn);

void updateUI(struct UI* ui);

void updatemap();

#endif/*VIEWINTERFACE_SCENE_H*/
