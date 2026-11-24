#include "UI/Players_UI.h"

#include "UI/UIs_info.h"
#include "DisplaySystem/display_system.h"
#include "EventsSystem/event_system.h"
#include "UI/universal_callbacks.h"

static void on_side_menu_btn_clicked(GtkButton* btn, gpointer userdata);
static void on_side_hide_btn_clicked(GtkButton* btn, gpointer userdata);

void create_Players_UI(gpointer userdata)
{
  struct Players_UI* players_ui = (struct Players_UI*)calloc(1, sizeof(struct Players_UI));
  /*Players builder*/
  GError* err = nullptr;
  GtkBuilder* pls_builder = gtk_builder_new();
  gtk_builder_add_from_file(pls_builder, PLAYERS_SOURCES, &err);
  if(err){
    g_print("Players page builder error: %s", err->message);
    g_object_unref(pls_builder);
    g_error_free(err);
    return;
  }
  players_ui->side_menu_revealer = GTK_REVEALER(gtk_builder_get_object(pls_builder, "side_menu_revealer"));
  players_ui->side_box_for_btn = GTK_BOX(gtk_builder_get_object(pls_builder, "side_box_for_btn"));
  players_ui->side_menu_btn = GTK_BUTTON(gtk_builder_get_object(pls_builder, "side_menu_btn"));
  players_ui->overlay = GTK_WIDGET(gtk_builder_get_object(pls_builder, "overlay"));
  players_ui->drag_btn = GTK_WIDGET(gtk_builder_get_object(pls_builder, "drag_btn"));
  g_object_set_data(G_OBJECT(userdata), "players_ui_data", players_ui);
  g_object_set_data(G_OBJECT(players_ui->overlay), "drag_btn", players_ui->drag_btn);
  /*Set gl_area tick callback*/
  GtkWidget* gl_area = GTK_WIDGET(gtk_builder_get_object(pls_builder, "gl_area"));
  gtk_widget_add_tick_callback(gl_area, ontick, userdata, nullptr);
  g_object_set_data(G_OBJECT(userdata), "gl_area", gl_area);
  /*Manualy set signals*/
  g_signal_connect(gl_area, "realize", G_CALLBACK(onrealize), userdata);
  g_signal_connect(gl_area, "unrealize", G_CALLBACK(onunrealize), userdata);
  g_signal_connect(gl_area, "render", G_CALLBACK(onrender), userdata);
  g_signal_connect(players_ui->side_menu_btn, "clicked", G_CALLBACK(on_side_menu_btn_clicked), userdata);
  GtkWidget* side_hide_btn = GTK_WIDGET(gtk_builder_get_object(pls_builder, "side_hide_btn"));
  g_signal_connect(side_hide_btn, "clicked", G_CALLBACK(on_side_hide_btn_clicked), userdata);
  GtkGesture* longpress = gtk_gesture_drag_new();
  g_signal_connect(longpress, "drag-begin", G_CALLBACK(on_drag_begin), userdata);
  g_signal_connect(longpress, "drag-end", G_CALLBACK(on_drag_end), userdata);
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(longpress), 0);
  gtk_widget_add_controller(players_ui->overlay, GTK_EVENT_CONTROLLER(longpress));
  /*Set drag btn active*/
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(players_ui->drag_btn), TRUE);
  /*Set scroll controller*/
  GtkEventController* scroll = gtk_event_controller_scroll_new(
    GTK_EVENT_CONTROLLER_SCROLL_VERTICAL
  );
  g_signal_connect(scroll, "scroll", G_CALLBACK(onscroll), userdata);
  gtk_widget_add_controller(players_ui->overlay, GTK_EVENT_CONTROLLER(scroll));
  /*Set motion controller*/
  GtkEventController* motion = gtk_event_controller_motion_new();
  g_signal_connect(motion, "motion", G_CALLBACK(onmotion), userdata);
  gtk_widget_add_controller(players_ui->overlay, motion);
  /*Set key controller*/
  GtkEventController* keycontroller = gtk_event_controller_key_new();
  g_signal_connect(keycontroller, "key-pressed", G_CALLBACK(on_key_pressed), userdata);
  g_signal_connect(keycontroller, "key-released", G_CALLBACK(on_key_released), userdata);
  g_signal_connect(keycontroller, "key-pressed", G_CALLBACK(on_escape_pressed), userdata);
  gtk_widget_add_controller(players_ui->overlay, keycontroller);
  /*Set double click controller*/
  GtkGesture* doubleclick = gtk_gesture_click_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(doubleclick), GDK_BUTTON_PRIMARY);
  g_signal_connect(doubleclick, "pressed", G_CALLBACK(on_click_pressed), userdata);
  g_signal_connect(doubleclick, "released", G_CALLBACK(on_click_released), userdata);
  gtk_widget_add_controller(players_ui->overlay, GTK_EVENT_CONTROLLER(doubleclick));
  /*Switch to players page*/
  GtkStack* interfacestack = g_object_get_data(G_OBJECT(userdata), "interfacestack");
  GtkWidget* players_page = GTK_WIDGET(gtk_builder_get_object(pls_builder, "players_page"));
  gtk_stack_add_named(interfacestack, players_page, "players_page");
  gtk_stack_set_visible_child_name(interfacestack, "players_page");
  /*Set ISDM = 0*/
  g_object_set_data(G_OBJECT(userdata), "ISDM", GINT_TO_POINTER(0));
  /*Unref builder*/
  g_object_unref(pls_builder);
}

/*Side menu btns*/
static void on_side_menu_btn_clicked(GtkButton* btn, gpointer userdata)
{
  gtk_widget_set_visible(GTK_WIDGET(btn), false);

  struct Players_UI* players_ui = (struct Players_UI*)g_object_get_data(G_OBJECT(userdata), "players_ui_data");
  gtk_revealer_set_reveal_child(players_ui->side_menu_revealer, true);
  gtk_widget_set_can_target(GTK_WIDGET(players_ui->side_box_for_btn), false);
}

static void on_side_hide_btn_clicked(GtkButton* btn, gpointer userdata)
{
  struct Players_UI* players_ui = (struct Players_UI*)g_object_get_data(G_OBJECT(userdata), "players_ui_data");
  gtk_revealer_set_reveal_child(players_ui->side_menu_revealer, false);
  gtk_widget_set_can_target(GTK_WIDGET(players_ui->side_box_for_btn), true);
  gtk_widget_set_visible(GTK_WIDGET(players_ui->side_menu_btn), true);
}
