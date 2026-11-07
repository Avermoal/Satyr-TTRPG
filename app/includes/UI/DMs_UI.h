#ifndef UI_DMS_UI_H
#define UI_DMS_UI_H

#include <gtk/gtk.h>

struct DMs_UI{
  GtkRevealer* side_menu_revealer;
  GtkBox* side_box_for_btn;
  GtkButton* side_menu_btn;
  GtkWidget* overlay;
  GtkWidget* tokens_box;
  GtkWidget* tokens_btn;
  GtkWidget* maps_btn;
  GtkWidget* drag_btn;
  GtkWidget* settings_btn;
  GtkWidget* settings_grid;
  GtkWidget* layers_box;
};

void create_DMs_UI(int ISNEW, gpointer userdata);

#endif/*UI_DMS_UI_H*/
