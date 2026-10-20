#include "UI/UI.h"

#include <stdlib.h>

#include "UI/UIs_info.h"
#include "UI/Menu_UI.h"
#include "UI/DMs_UI.h"
#include "UI/Players_UI.h"
#include "DisplaySystem/display_system.h"
#include "EventsSystem/file_dnd.h"

#define MENU_SOURCES "res/gtk_resources_pages/menu_page.ui"
#define DMS_SOURCES "res/gtk_resources_pages/dms_page.ui"
#define PLAYERS_SOURCES "res/gtk_resources_pages/players_page.ui"
#define NEW_CAMPAIGN_SOURCES "res/gtk_resources_pages/new_campaign_window.ui"

static void set_dms_page_callbacks(GtkBuilder* builder, gpointer dms_ui, GtkWindow* win);
static void set_menu_page_callbacks(GtkBuilder* builder, GtkWindow* win);
static void set_new_campaign_page_callbacks(GtkBuilder* builder, GtkWindow* win);

void createUI(struct UI* ui, GtkWindow* win)
{
  if(!ui || !win){
    return;
  }
  /*Get interfacestack*/
  GtkStack* interfacestack = g_object_get_data(G_OBJECT(win), "interfacestack");
  /*Allocation memory*/
  ui->dms_ui = (struct DMs_UI*)calloc(1, sizeof(struct DMs_UI));
  /*Loading ui's files (get builders) and expose objects for callbacks*/
    /*Menu*/
  GtkBuilder* menu_builder = gtk_builder_new();
  gtk_builder_expose_object(menu_builder, "win", G_OBJECT(win));
  set_menu_page_callbacks(menu_builder, win);
  gtk_builder_add_from_file(menu_builder, MENU_SOURCES, nullptr);
    /*DMs*/
  GtkBuilder* dms_builder = gtk_builder_new();
  set_dms_page_callbacks(dms_builder, ui->dms_ui, win);
  gtk_builder_add_from_file(dms_builder, DMS_SOURCES, nullptr);
    /*Players*/
  GtkBuilder* players_builder = gtk_builder_new();
  gtk_builder_expose_object(players_builder, "win", G_OBJECT(win));
  gtk_builder_add_from_file(players_builder, PLAYERS_SOURCES, nullptr);
    /*New campaign*/
  GtkBuilder* new_campagin_builder = gtk_builder_new();
  set_new_campaign_page_callbacks(new_campagin_builder, win);
  gtk_builder_add_from_file(new_campagin_builder, NEW_CAMPAIGN_SOURCES, nullptr);
  /*Set DMs UI widgets*/
  ui->dms_ui->side_menu_revealer = GTK_REVEALER(gtk_builder_get_object(dms_builder, "side_menu_revealer"));
  ui->dms_ui->side_box_for_btn = GTK_BOX(gtk_builder_get_object(dms_builder, "side_box_for_btn"));
  ui->dms_ui->side_menu_btn = GTK_BUTTON(gtk_builder_get_object(dms_builder, "side_menu_btn"));
  /*Get root page's widgets*/
  ui->menu_page = GTK_WIDGET(gtk_builder_get_object(menu_builder, "menu_page"));
  ui->dms_page = GTK_WIDGET(gtk_builder_get_object(dms_builder, "dms_page"));
  ui->players_page = GTK_WIDGET(gtk_builder_get_object(players_builder, "players_page"));
  ui->new_campaign_page = GTK_WIDGET(gtk_builder_get_object(new_campagin_builder, "new_campaign_page"));
  gtk_stack_add_named(interfacestack, ui->menu_page, "menu_page");
  gtk_stack_add_named(interfacestack, ui->dms_page, "dms_page");
  gtk_stack_add_named(interfacestack, ui->players_page, "players_page");
  gtk_stack_add_named(interfacestack, ui->new_campaign_page, "new_campaign_page");
  /*Get Entry Buffer and set like obj data*/
  GtkEntryBuffer* entrybuffer = GTK_ENTRY_BUFFER(gtk_builder_get_object(new_campagin_builder, "campaign_name_buffer"));
  g_object_set_data(G_OBJECT(win), "campaign_name_buffer", entrybuffer);
  /*Present current UI*/
  gtk_stack_set_visible_child_name(interfacestack, "menu_page");
  /*Set GtkOpenGLArea ticker*/
  GtkWidget* gl_area = GTK_WIDGET(gtk_builder_get_object(dms_builder, "gl_area"));
  gtk_widget_add_tick_callback(gl_area, ontick, win, nullptr);
  g_object_set_data(G_OBJECT(win), "gl_area", gl_area);
  /*Set Drag and Drop files target*/
  GtkWidget* maingrid = GTK_WIDGET(gtk_builder_get_object(dms_builder, "main_grid"));
  GtkDropTarget* target = gtk_drop_target_new(GDK_TYPE_FILE_LIST, GDK_ACTION_COPY);
  g_signal_connect(target, "drop", G_CALLBACK(on_files_drop), win);
  gtk_widget_add_controller(maingrid, GTK_EVENT_CONTROLLER(target));
  /*Destroy builders*/
  g_object_unref(menu_builder);
  g_object_unref(dms_builder);
  g_object_unref(players_builder);
  g_object_unref(new_campagin_builder);
}

void destroyUI(struct UI* ui)
{
  free(ui->dms_ui);
}

static void set_dms_page_callbacks(GtkBuilder* builder, gpointer dms_ui, GtkWindow* win)
{
  g_object_set_data(G_OBJECT(win), "dms_ui_data", dms_ui);

  GtkBuilderScope* scope = gtk_builder_cscope_new();
  /*Side menu btn*/
  gtk_builder_cscope_add_callback_symbol(GTK_BUILDER_CSCOPE(scope), "on_side_menu_btn_clicked",
                                         G_CALLBACK(on_side_menu_btn_clicked));
  gtk_builder_cscope_add_callback_symbol(GTK_BUILDER_CSCOPE(scope), "on_side_hide_btn_clicked",
                                         G_CALLBACK(on_side_hide_btn_clicked));
  /*Top menu btn*/
  gtk_builder_cscope_add_callback_symbol(GTK_BUILDER_CSCOPE(scope), "on_scene_menu_btn_clicked",
                                         G_CALLBACK(on_scene_menu_btn_clicked));
  /*RENDER*/
  gtk_builder_cscope_add_callback_symbol(GTK_BUILDER_CSCOPE(scope), "realize",
                                         G_CALLBACK(onrealize));
  gtk_builder_cscope_add_callback_symbol(GTK_BUILDER_CSCOPE(scope), "unrealize",
                                         G_CALLBACK(onunrealize));
  gtk_builder_cscope_add_callback_symbol(GTK_BUILDER_CSCOPE(scope), "render",
                                         G_CALLBACK(onrender));

  gtk_builder_set_scope(builder, scope);
  gtk_builder_set_current_object(builder, G_OBJECT(win));
  g_object_unref(scope);
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
