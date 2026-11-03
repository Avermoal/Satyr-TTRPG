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
};

void on_side_menu_btn_clicked(GtkButton* btn, gpointer userdata);

void on_side_hide_btn_clicked(GtkButton* btn, gpointer userdata);

/*ESCAPE*/
gboolean on_escape_pressed(GtkEventControllerKey* controller, guint keyval,
                       guint keycode, GdkModifierType state, gpointer userdata);

/*Top menu*/
void on_tokens_btn_toggled(GtkToggleButton* btn, gpointer userdata);

void on_maps_btn_toggled(GtkToggleButton* btn, gpointer userdata);

#endif/*UI_DMS_UI_H*/
