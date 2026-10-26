#ifndef UI_DMS_UI_H
#define UI_DMS_UI_H

#include <gtk/gtk.h>

struct DMs_UI{
  GtkRevealer* side_menu_revealer;
  GtkBox* side_box_for_btn;
  GtkButton* side_menu_btn;
};

void on_side_menu_btn_clicked(GtkButton* btn, gpointer userdata);

void on_side_hide_btn_clicked(GtkButton* btn, gpointer userdata);

/*ESCAPE*/
void on_escape_pressed(GtkEventControllerKey* controller, guint keyval,
                       guint keycode, GdkModifierType state, gpointer userdata);

#endif/*UI_DMS_UI_H*/
