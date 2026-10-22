#include "UI/DMs_UI.h"

#include "UI/UI.h"

#define SCENE_MENU_UI "res/gtk_resources_pages/scene_menu.ui"
#define SCENE_UI "res/gtk_resources_pages/scene.ui"

/*Scene menu*/
static void on_file_open_ready(GObject* obj, GAsyncResult* res, gpointer userdata);

/*Side menu btns*/
void on_side_menu_btn_clicked(GtkButton* btn, gpointer userdata)
{
  gtk_widget_set_visible(GTK_WIDGET(btn), false);

  struct DMs_UI* dms_ui = (struct DMs_UI*)g_object_get_data(G_OBJECT(userdata), "dms_ui_data");
  gtk_revealer_set_reveal_child(dms_ui->side_menu_revealer, true);
  gtk_widget_set_can_target(GTK_WIDGET(dms_ui->side_box_for_btn), false);
}

void on_side_hide_btn_clicked(GtkButton* btn, gpointer userdata)
{
  struct DMs_UI* dms_ui = (struct DMs_UI*)g_object_get_data(G_OBJECT(userdata), "dms_ui_data");
  gtk_revealer_set_reveal_child(dms_ui->side_menu_revealer, false);
  gtk_widget_set_can_target(GTK_WIDGET(dms_ui->side_box_for_btn), true);
  gtk_widget_set_visible(GTK_WIDGET(dms_ui->side_menu_btn), true);
}
