#include "UI/Menu_UI.h"

#include <stdlib.h>
#include <string.h>

#include "UI/UIs_info.h"
#include "UI/DMs_UI.h"
#include "GameState/game_state.h"
#include "DisplaySystem/display_system.h"
#include "EventsSystem/file_dnd.h"

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
  GtkWindow* win = GTK_WINDOW(userdata);
  /*CREATE dms_page UI*/
    /*Allocation memory*/
  struct DMs_UI* dms_ui = (struct DMs_UI*)calloc(1, sizeof(struct DMs_UI));
    /*DMs*/
  GtkBuilder* dms_builder = gtk_builder_new();
  gtk_builder_add_from_file(dms_builder, DMS_SOURCES, nullptr);
  /*Set DMs UI widgets*/
  dms_ui->side_menu_revealer = GTK_REVEALER(gtk_builder_get_object(dms_builder, "side_menu_revealer"));
  dms_ui->side_box_for_btn = GTK_BOX(gtk_builder_get_object(dms_builder, "side_box_for_btn"));
  dms_ui->side_menu_btn = GTK_BUTTON(gtk_builder_get_object(dms_builder, "side_menu_btn"));
  g_object_set_data(G_OBJECT(userdata), "dms_ui_data", dms_ui);
  /*Set GtkOpenGLArea ticker*/
  GtkWidget* gl_area = GTK_WIDGET(gtk_builder_get_object(dms_builder, "gl_area"));
  gtk_widget_add_tick_callback(gl_area, ontick, win, nullptr);
  g_object_set_data(G_OBJECT(win), "gl_area", gl_area);
  /*Manualy set signals*/
  g_signal_connect(gl_area, "realize", G_CALLBACK(onrealize), userdata);
  g_signal_connect(gl_area, "unrealize", G_CALLBACK(onunrealize), userdata);
  g_signal_connect(gl_area, "render", G_CALLBACK(onrender), userdata);
  GtkWidget* side_menu_btn = GTK_WIDGET(gtk_builder_get_object(dms_builder, "side_menu_btn"));
  g_signal_connect(side_menu_btn, "clicked", G_CALLBACK(on_side_menu_btn_clicked), userdata);
  GtkWidget* side_hide_btn = GTK_WIDGET(gtk_builder_get_object(dms_builder, "side_hide_btn"));
  g_signal_connect(side_hide_btn, "clicked", G_CALLBACK(on_side_hide_btn_clicked), userdata);
  GtkWidget* tokens_box = GTK_WIDGET(gtk_builder_get_object(dms_builder, "tokens_box"));
  g_signal_connect(tokens_box, "map", G_CALLBACK(on_tokens_box_map), userdata);
  g_signal_connect(tokens_box, "unmap", G_CALLBACK(on_tokens_box_unmap), userdata);
  /*Set Drag and Drop files target*/
    /*Content formats*/
  GdkContentFormatsBuilder* formatsbuilder = gdk_content_formats_builder_new();
  gdk_content_formats_builder_add_mime_type(formatsbuilder, "text/uri-list");
  GdkContentFormats* formats = gdk_content_formats_builder_free_to_formats(formatsbuilder);
    /*Set*/
  GtkWidget* overlay = GTK_WIDGET(gtk_builder_get_object(dms_builder, "dms_page"));
  GtkDropTargetAsync* target = gtk_drop_target_async_new(
    gdk_content_formats_new((const char* []){"text/uri-list", "text/plain"}, 2), GDK_ACTION_COPY);
  g_signal_connect(target, "drop", G_CALLBACK(on_toks_drop), userdata);
  g_signal_connect(target, "drag-enter", G_CALLBACK(on_toks_drag_enter), userdata);
  g_signal_connect(target, "drag-leave", G_CALLBACK(on_toks_drag_leave), userdata);
  gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(target), GTK_PHASE_CAPTURE);
  gtk_widget_add_controller(overlay, GTK_EVENT_CONTROLLER(target));
    /*Free formats*/
  gdk_content_formats_unref(formats);
  /*Get entrybuffer with game name*/
  GtkEntryBuffer* entrybuffer = GTK_ENTRY_BUFFER(g_object_get_data(G_OBJECT(userdata), "campaign_name_buffer"));
  g_object_set_data(G_OBJECT(userdata), "entrybuffer", entrybuffer);
  /*Switch to dms_page*/
  GtkStack* interfacestack = g_object_get_data(G_OBJECT(userdata), "interfacestack");
  GtkWidget* dms_page = GTK_WIDGET(gtk_builder_get_object(dms_builder, "dms_page"));
  gtk_stack_add_named(interfacestack, dms_page, "dms_page");
  gtk_stack_set_visible_child_name(interfacestack, "dms_page");
  /*Unref builder*/
  g_object_unref(dms_builder);
}

void on_back_btn_clicked(GtkButton* btn, gpointer userdata)
{
  GtkStack* interfacestack = g_object_get_data(G_OBJECT(userdata), "interfacestack");
  gtk_stack_set_visible_child_name(interfacestack, "menu_page");
}
