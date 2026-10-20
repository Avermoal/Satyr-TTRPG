#ifndef UI_UI_H
#define UI_UI_H

#include <gtk/gtk.h>

struct DMs_UI{
  GtkRevealer* side_menu_revealer;
  GtkBox* side_box_for_btn;
  GtkButton* side_menu_btn;
};

struct UI{
  struct DMs_UI* dms_ui;
  GtkWidget* menu_page;
  GtkWidget* dms_page;
  GtkWidget* players_page;
  GtkWidget* new_campaign_page;
};

void createUI(struct UI* ui, GtkWindow* win);

void destroyUI(struct UI* ui);

#endif/*UI_UI_H*/
