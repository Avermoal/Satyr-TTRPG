#include "UI/UI.h"

#include <stdlib.h>

#include "UI/UIs_info.h"
#include "UI/Menu_UI.h"
#include "UI/DMs_UI.h"
#include "UI/Players_UI.h"

static void set_menu_page_callbacks(GtkBuilder* builder, GtkWindow* win);
static void set_new_campaign_page_callbacks(GtkBuilder* builder, GtkWindow* win);

void createUI(struct UI* ui, GtkWindow* win)
{
  if(!ui || !win){
    return;
  }
  /*Get interfacestack*/
  GtkStack* interfacestack = g_object_get_data(G_OBJECT(win), "interfacestack"); 
  /*Loading ui's files (get builders) and expose objects for callbacks*/
    /*Menu*/
  GtkBuilder* menu_builder = gtk_builder_new();
  gtk_builder_expose_object(menu_builder, "win", G_OBJECT(win));
  set_menu_page_callbacks(menu_builder, win);
  gtk_builder_add_from_file(menu_builder, MENU_SOURCES, nullptr);
    /*Players*/
  GtkBuilder* players_builder = gtk_builder_new();
  gtk_builder_expose_object(players_builder, "win", G_OBJECT(win));
  gtk_builder_add_from_file(players_builder, PLAYERS_SOURCES, nullptr);
    /*New campaign*/
  GtkBuilder* new_campagin_builder = gtk_builder_new();
  set_new_campaign_page_callbacks(new_campagin_builder, win);
  gtk_builder_add_from_file(new_campagin_builder, NEW_CAMPAIGN_SOURCES, nullptr);
  /*Get root page's widgets*/
  ui->menu_page = GTK_WIDGET(gtk_builder_get_object(menu_builder, "menu_page"));
  ui->players_page = GTK_WIDGET(gtk_builder_get_object(players_builder, "players_page"));
  ui->new_campaign_page = GTK_WIDGET(gtk_builder_get_object(new_campagin_builder, "new_campaign_page"));
  gtk_stack_add_named(interfacestack, ui->menu_page, "menu_page");
  gtk_stack_add_named(interfacestack, ui->players_page, "players_page");
  gtk_stack_add_named(interfacestack, ui->new_campaign_page, "new_campaign_page");
  /*Get Entry Buffer and set like obj data*/
  GtkEntryBuffer* entrybuffer = GTK_ENTRY_BUFFER(gtk_builder_get_object(new_campagin_builder, "campaign_name_buffer"));
  g_object_set_data(G_OBJECT(win), "campaign_name_buffer", entrybuffer);
  /*Present current UI*/
  gtk_stack_set_visible_child_name(interfacestack, "menu_page");
  /*Destroy builders*/
  g_object_unref(menu_builder);
  g_object_unref(players_builder);
  g_object_unref(new_campagin_builder);
}

static void set_menu_page_callbacks(GtkBuilder* builder, GtkWindow* win)
{
  GtkBuilderScope* scope = gtk_builder_cscope_new();

  gtk_builder_cscope_add_callback_symbol(GTK_BUILDER_CSCOPE(scope), "on_to_new_campaign_btn_clicked",
                                         G_CALLBACK(on_to_new_campaign_btn_clicked));
  gtk_builder_cscope_add_callback_symbol(GTK_BUILDER_CSCOPE(scope), "on_to_load_save_btn_clicked",
                                         G_CALLBACK(on_to_load_save_btn_clicked));
  gtk_builder_cscope_add_callback_symbol(GTK_BUILDER_CSCOPE(scope), "on_to_join_btn_clicked",
                                         G_CALLBACK(on_to_join_btn_clicked));
  gtk_builder_cscope_add_callback_symbol(GTK_BUILDER_CSCOPE(scope), "on_exit_btn_clicked",
                                         G_CALLBACK(on_exit_btn_clicked));
  gtk_builder_set_scope(builder, scope);
  gtk_builder_set_current_object(builder, G_OBJECT(win));
  g_object_unref(scope);
}

static void set_new_campaign_page_callbacks(GtkBuilder* builder, GtkWindow* win)
{
  GtkBuilderScope* scope = gtk_builder_cscope_new();

  gtk_builder_cscope_add_callback_symbol(GTK_BUILDER_CSCOPE(scope), "on_letsgo_btn_clicked",
                                         G_CALLBACK(on_letsgo_btn_clicked));
  gtk_builder_cscope_add_callback_symbol(GTK_BUILDER_CSCOPE(scope), "on_back_btn_clicked",
                                         G_CALLBACK(on_back_btn_clicked));
  gtk_builder_set_scope(builder, scope);
  gtk_builder_set_current_object(builder, G_OBJECT(win));
  g_object_unref(scope);

}
