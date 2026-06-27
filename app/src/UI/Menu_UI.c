#include "UI/Menu_UI.h"

void on_to_new_campain_btn_clicked(GtkButton* btn, GtkStack* interfacestack, gpointer userdata)
{
  gtk_stack_set_visible_child_name(interfacestack, "dms_page");
}

void on_to_load_save_btn_clicked(GtkButton* btn, GtkStack* interfacestack, gpointer userdata)
{

}

void on_to_join_btn_clicked(GtkButton* btn, GtkStack* interfacestack, gpointer userdata)
{

}

void on_exit_btn_clicked(GtkButton* btn, GtkWindow* win, gpointer userdata)
{
  gtk_window_destroy(win);
}
