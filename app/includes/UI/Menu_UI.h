#ifndef UI_MENU_UI
#define UI_MENU_UI

#include <gtk/gtk.h>

/*Callbacks*/
void on_to_new_campain_btn_clicked(GtkButton* btn, GtkStack* interfacestack, gpointer userdata);

void on_to_load_save_btn_clicked(GtkButton* btn, GtkStack* interfacestack, gpointer userdata);

void on_to_join_btn_clicked(GtkButton* btn, GtkStack* interfacestack, gpointer userdata);

void on_exit_btn_clicked(GtkButton* btn, GtkWindow* win, gpointer userdata);

#endif/*UI_MENU_UI*/
