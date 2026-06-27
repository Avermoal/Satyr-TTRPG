#ifndef UI_UI_H
#define UI_UI_H

#include <gtk/gtk.h>

struct UI{
  GtkWidget* menu_page;
  GtkBuilder* menu_builder;
  GtkWidget* dms_page;
  GtkBuilder* dms_builder;
  GtkWidget* players_page;
  GtkBuilder* players_builder;
  short WHICHUI;
};

void createUI(struct UI* ui, GtkWindow* win);

void destroyUI(struct UI* ui);

#endif/*UI_UI_H*/
