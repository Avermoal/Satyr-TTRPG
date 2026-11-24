#ifndef UI_PALYERS_UI_H
#define UI_PALYERS_UI_H

#include <gtk/gtk.h>

struct Players_UI{
  GtkRevealer* side_menu_revealer;
  GtkBox* side_box_for_btn;
  GtkButton* side_menu_btn;
  GtkWidget* overlay;
  GtkWidget* drag_btn;
};

void create_Players_UI(gpointer userdata);

#endif/*UI_PALYERS_UI_H*/
