#ifndef UI_MENU_UI
#define UI_MENU_UI

#include <gtk/gtk.h>

/*Callbacks*/
void on_to_new_campaign_btn_clicked(GtkButton* btn, gpointer userdata);

void on_to_load_save_btn_clicked(GtkButton* btn, gpointer userdata);

void on_to_join_btn_clicked(GtkButton* btn, gpointer userdata);

void on_exit_btn_clicked(GtkButton* btn, gpointer userdata);

#endif/*UI_MENU_UI*/
