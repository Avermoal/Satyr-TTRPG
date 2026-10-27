#include "UI/DMs_UI.h"

/*Side menu btns*/
G_MODULE_EXPORT void on_side_menu_btn_clicked(GtkButton* btn, gpointer userdata)
{
  gtk_widget_set_visible(GTK_WIDGET(btn), false);

  struct DMs_UI* dms_ui = (struct DMs_UI*)g_object_get_data(G_OBJECT(userdata), "dms_ui_data");
  gtk_revealer_set_reveal_child(dms_ui->side_menu_revealer, true);
  gtk_widget_set_can_target(GTK_WIDGET(dms_ui->side_box_for_btn), false);
}

G_MODULE_EXPORT void on_side_hide_btn_clicked(GtkButton* btn, gpointer userdata)
{
  struct DMs_UI* dms_ui = (struct DMs_UI*)g_object_get_data(G_OBJECT(userdata), "dms_ui_data");
  gtk_revealer_set_reveal_child(dms_ui->side_menu_revealer, false);
  gtk_widget_set_can_target(GTK_WIDGET(dms_ui->side_box_for_btn), true);
  gtk_widget_set_visible(GTK_WIDGET(dms_ui->side_menu_btn), true);
}

/*ESCAPE*/
G_MODULE_EXPORT void on_escape_pressed(GtkEventControllerKey* controller, guint keyval,
                       guint keycode, GdkModifierType state, gpointer userdata)
{
  if(keyval != GDK_KEY_Escape){
    return;
  }
  /*Get dms_ui pointer*/
  struct DMs_UI* dms_ui = (struct DMs_UI*)g_object_get_data(G_OBJECT(userdata), "dms_ui_data");
  /*Free*/
  free(dms_ui);
  dms_ui = nullptr;
  /*Get interfacestack*/
  GtkStack* interfacestack = GTK_STACK(g_object_get_data(G_OBJECT(userdata), "interfacestack"));
  /*Set visible menu*/
  gtk_stack_set_visible_child_name(interfacestack, "menu_page");
  /*Remove dms_page from interfacestack*/
  GtkWidget* dms_page = GTK_WIDGET(gtk_stack_get_child_by_name(interfacestack, "dms_page"));
  gtk_stack_remove(interfacestack, dms_page);
  /*Remove on_escape_pressed callback*/
  g_signal_handlers_disconnect_by_func(controller, G_CALLBACK(on_escape_pressed), userdata);
}
