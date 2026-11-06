#include "UI/UI.h"

#include <stdlib.h>

#include "UI/UIs_info.h"
#include "UI/Menu_UI.h"

static void set_menu_page_callbacks(GtkBuilder* builder, GtkWindow* win);

void createUI(struct UI* ui, GtkWindow* win)
{
  if(!ui || !win){
    return;
  }
  /*Get interfacestack*/
  GtkStack* interfacestack = g_object_get_data(G_OBJECT(win), "interfacestack"); 
  /*Loading ui's files (get builders) and expose objects for callbacks*/
    /*Menu*/
  GError* err = nullptr;
  GtkBuilder* menu_builder = gtk_builder_new();
  gtk_builder_expose_object(menu_builder, "win", G_OBJECT(win));
  set_menu_page_callbacks(menu_builder, win);
  gtk_builder_add_from_file(menu_builder, MENU_SOURCES, &err);
  if(err){
    g_print("Menu builder error: %s\n", err->message);
    g_object_unref(menu_builder);
    g_error_free(err);
    return;
  }
  /*Get root page widgets*/
  ui->menu_page = GTK_WIDGET(gtk_builder_get_object(menu_builder, "menu_page"));
  gtk_stack_add_named(interfacestack, ui->menu_page, "menu_page");
  /*Present current UI*/
  gtk_stack_set_visible_child_name(interfacestack, "menu_page");
  /*Destroy builder*/
  g_object_unref(menu_builder);
}

void change_widget_cursor(GtkWidget* widget, const char* name)
{
  GdkCursor *cursor = gdk_cursor_new_from_name(name, NULL);
  gtk_widget_set_cursor(widget, cursor);
  g_object_unref(cursor);
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
