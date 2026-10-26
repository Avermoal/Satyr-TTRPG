#ifndef UI_UI_H
#define UI_UI_H

#include <gtk/gtk.h>

struct UI{
  GtkWidget* menu_page;
  GtkWidget* players_page;
  GtkWidget* new_campaign_page;
};

void createUI(struct UI* ui, GtkWindow* win);

#endif/*UI_UI_H*/
