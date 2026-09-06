#include "UI/UI.h"

#include <stdlib.h>

#include "UI/UIs_info.h"
#include "UI/Menu_UI.h"
#include "UI/DMs_UI.h"
#include "UI/Players_UI.h"

#define MENU_SOURCES "res/gtk_resources_pages/menu_page.ui"
#define DMS_SOURCES "res/gtk_resources_pages/dms_page.ui"
#define PLAYERS_SOURCES "res/gtk_resources_pages/players_page.ui"

static void set_dms_page_callbacks(GtkBuilder* builder, gpointer dms_ui, GtkWindow* win);

void createUI(struct UI* ui, GtkWindow* win)
{
  if(ui){
    /*Get interfacestack*/
    GtkStack* interfacestack = g_object_get_data(G_OBJECT(win), "interfacestack");
    /*Allocation memory for DMs_UI*/
    ui->dms_ui = g_new0(struct DMs_UI, 1);
    /*Loading ui's files (get builders) and expose objects for callbacks*/
    ui->menu_builder = gtk_builder_new();
    gtk_builder_expose_object(ui->menu_builder, "win", G_OBJECT(win));
    gtk_builder_expose_object(ui->menu_builder, "interfacestack", G_OBJECT(interfacestack));
    gtk_builder_add_from_file(ui->menu_builder, MENU_SOURCES, nullptr);
    ui->dms_builder = gtk_builder_new();
    set_dms_page_callbacks(ui->dms_builder, ui->dms_ui, win);
    gtk_builder_add_from_file(ui->dms_builder, DMS_SOURCES, nullptr);
    ui->players_builder = gtk_builder_new();
    gtk_builder_expose_object(ui->players_builder, "win", G_OBJECT(win));
    gtk_builder_add_from_file(ui->players_builder, PLAYERS_SOURCES, nullptr);
    /*Set DMs UI widgets*/
    ui->dms_ui->side_menu_revealer = GTK_REVEALER(gtk_builder_get_object(ui->dms_builder, "side_menu_revealer"));
    ui->dms_ui->side_box_for_btn = GTK_BOX(gtk_builder_get_object(ui->dms_builder, "side_box_for_btn"));
    ui->dms_ui->side_menu_btn = GTK_BUTTON(gtk_builder_get_object(ui->dms_builder, "side_menu_btn"));
    /*Get root page's widgets*/
    ui->menu_page = GTK_WIDGET(gtk_builder_get_object(ui->menu_builder, "menu_page"));
    ui->dms_page = GTK_WIDGET(gtk_builder_get_object(ui->dms_builder, "dms_page"));
    ui->players_page = GTK_WIDGET(gtk_builder_get_object(ui->players_builder, "players_page"));
    gtk_stack_add_named(interfacestack, ui->menu_page, "menu_page");
    gtk_stack_add_named(interfacestack, ui->dms_page, "dms_page");
    gtk_stack_add_named(interfacestack, ui->players_page, "players_page");
    /*Present current UI*/
    ui->WHICHUI = IS_MENU;
    gtk_stack_set_visible_child_name(interfacestack, "menu_page");
  }
}

void destroyUI(struct UI* ui)
{
  g_object_unref(ui->menu_builder);
  g_object_unref(ui->dms_builder);
  g_object_unref(ui->players_builder);
  free(ui);
}

static void set_dms_page_callbacks(GtkBuilder* builder, gpointer dms_ui, GtkWindow* win)
{
  g_object_set_data(G_OBJECT(win), "dms_ui_data", dms_ui);

  GtkBuilderScope* scope = gtk_builder_cscope_new();

  gtk_builder_cscope_add_callback_symbol(GTK_BUILDER_CSCOPE(scope), "on_side_menu_btn_clicked",
                                         G_CALLBACK(on_side_menu_btn_clicked));
  gtk_builder_cscope_add_callback_symbol(GTK_BUILDER_CSCOPE(scope), "on_side_hide_btn_clicked",
                                         G_CALLBACK(on_side_hide_btn_clicked));
  gtk_builder_set_scope(builder, scope);
  gtk_builder_set_current_object(builder, G_OBJECT(win));
}
