#include "UI/Menu_UI.h"

#include <stdlib.h>
#include <string.h>

#include "GameState/game_state.h"

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
  /*Get gamestate pointer*/
  struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(userdata), "gstate");
  /*Set campaign name*/
  GtkEntryBuffer* entrybuffer = GTK_ENTRY_BUFFER(g_object_get_data(G_OBJECT(userdata), "campaign_name_buffer"));
  gstate->gamename = strdup(gtk_entry_buffer_get_text(entrybuffer));
  /*Switch to dms_page*/
  GtkStack* interfacestack = g_object_get_data(G_OBJECT(userdata), "interfacestack");
  gtk_stack_set_visible_child_name(interfacestack, "dms_page");
}

void on_back_btn_clicked(GtkButton* btn, gpointer userdata)
{
  GtkStack* interfacestack = g_object_get_data(G_OBJECT(userdata), "interfacestack");
  gtk_stack_set_visible_child_name(interfacestack, "menu_page");
}
