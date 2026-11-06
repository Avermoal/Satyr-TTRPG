#include "UI/Menu_UI.h"

#include <stdlib.h>
#include <string.h>

#include "UI/DMs_UI.h"

void on_to_new_campaign_btn_clicked(GtkButton* btn, gpointer userdata)
{
  GtkStack* interfacestack = g_object_get_data(G_OBJECT(userdata), "interfacestack");
  gtk_stack_set_visible_child_name(interfacestack, "new_campaign_page");
}

void on_to_load_save_btn_clicked(GtkButton* btn, gpointer userdata)
{

}

void on_to_join_btn_clicked(GtkButton* btn, gpointer userdata)
{

}

void on_exit_btn_clicked(GtkButton* btn, gpointer userdata)
{
  gtk_window_destroy(GTK_WINDOW(userdata));
}

void on_letsgo_btn_clicked(GtkButton* btn, gpointer userdata)
{
  create_DMs_UI(1, userdata);
}

void on_back_btn_clicked(GtkButton* btn, gpointer userdata)
{
  GtkStack* interfacestack = g_object_get_data(G_OBJECT(userdata), "interfacestack");
  gtk_stack_set_visible_child_name(interfacestack, "menu_page");
}
