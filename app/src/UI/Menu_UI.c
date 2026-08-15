#include "UI/Menu_UI.h"

void on_to_new_campaign_btn_clicked(GtkButton* btn, gpointer userdata)
{
  gtk_stack_set_visible_child_name(GTK_STACK(userdata), "dms_page");
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
