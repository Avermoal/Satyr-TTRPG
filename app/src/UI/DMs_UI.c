#include "UI/DMs_UI.h"

#include <stdint.h>
#include <string.h>

#include "GameState/game_state.h"
#include "Tokens/tokens.h"
#include "EventsSystem/token_dnd.h"

/*Side menu btns*/
void on_side_menu_btn_clicked(GtkButton* btn, gpointer userdata)
{
  gtk_widget_set_visible(GTK_WIDGET(btn), false);

  struct DMs_UI* dms_ui = (struct DMs_UI*)g_object_get_data(G_OBJECT(userdata), "dms_ui_data");
  gtk_revealer_set_reveal_child(dms_ui->side_menu_revealer, true);
  gtk_widget_set_can_target(GTK_WIDGET(dms_ui->side_box_for_btn), false);
}

void on_side_hide_btn_clicked(GtkButton* btn, gpointer userdata)
{
  struct DMs_UI* dms_ui = (struct DMs_UI*)g_object_get_data(G_OBJECT(userdata), "dms_ui_data");
  gtk_revealer_set_reveal_child(dms_ui->side_menu_revealer, false);
  gtk_widget_set_can_target(GTK_WIDGET(dms_ui->side_box_for_btn), true);
  gtk_widget_set_visible(GTK_WIDGET(dms_ui->side_menu_btn), true);
}

/*ESCAPE*/
gboolean on_escape_pressed(GtkEventControllerKey* controller, guint keyval,
                       guint keycode, GdkModifierType state, gpointer userdata)
{
  if(keyval != GDK_KEY_Escape){
    return FALSE;
  }
  /*Get interfacestack*/
  GtkStack* interfacestack = GTK_STACK(g_object_get_data(G_OBJECT(userdata), "interfacestack"));
  /*Set visible menu*/
  gtk_stack_set_visible_child_name(interfacestack, "menu_page");
  /*Remove dms_page from interfacestack*/
  GtkWidget* dms_page = GTK_WIDGET(gtk_stack_get_child_by_name(interfacestack, "dms_page"));
  gtk_stack_remove(interfacestack, dms_page);
  /*DMs_UI free*/
  struct DMs_UI* dms_ui = (struct DMs_UI*)g_object_get_data(G_OBJECT(userdata), "dms_ui_data");
  free(dms_ui);
  dms_ui = nullptr;
  g_object_set_data(G_OBJECT(userdata), "dms_ui_data", nullptr);
  /*Remove on_escape_pressed callback*/
  g_signal_handlers_disconnect_by_func(controller, G_CALLBACK(on_escape_pressed), userdata);
  return TRUE;
}

/*Top menu*/
void on_tokens_btn_toggled(GtkToggleButton* btn, gpointer userdata)
{
  struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(userdata), "gstate");
  struct DMs_UI* dms_ui = (struct DMs_UI*)g_object_get_data(G_OBJECT(userdata), "dms_ui_data");
  /*Fill/clear tokens box*/
  if(gtk_toggle_button_get_active(btn)){
    /*Add tokens to box*/
      /*Switch maps btn to FALSE*/
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dms_ui->maps_btn), FALSE);
    if(gstate->tnum == 0){
      return;
    }
    for(int32_t i = 0; i < gstate->tnum; ++i){
      /*Create widget*/
      GdkTexture* texture = gdk_texture_new_from_filename(gstate->tokens[i].path, nullptr);
      GtkWidget* image = gtk_image_new_from_paintable(GDK_PAINTABLE(texture));
      tokensdata* tdata = (tokensdata*)g_new0(tokensdata, 1);
      if(!tdata){
        continue;
      }
      tdata->id = i;
      tdata->type = TOKEN;
      /*Add tokens image to box*/
      gtk_box_append(GTK_BOX(dms_ui->tokens_box), image);
      /*Drag and drop prepare settings*/
      GtkDragSource* drag_source = gtk_drag_source_new();
      g_signal_connect_data(drag_source, "prepare", G_CALLBACK(on_token_drag_prepare),
                            (gpointer)tdata, (GClosureNotify)(g_free), 0);
      g_signal_connect(drag_source, "drag-begin", G_CALLBACK(on_token_drag_begin), image);
      /*Set dnd controller*/
      gtk_widget_add_controller(image, GTK_EVENT_CONTROLLER(drag_source));
      /*Free*/
      g_object_unref(texture);
    }
    gtk_widget_set_visible(dms_ui->tokens_box, TRUE);
  }else{
    /*Clear box*/
    GtkWidget* child;
    while((child = gtk_widget_get_first_child(dms_ui->tokens_box))){
      gtk_box_remove(GTK_BOX(dms_ui->tokens_box), child);
    }
    gtk_widget_set_visible(dms_ui->tokens_box, FALSE);
  }
}

void on_maps_btn_toggled(GtkToggleButton* btn, gpointer userdata)
{
  struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(userdata), "gstate");
  struct DMs_UI* dms_ui = (struct DMs_UI*)g_object_get_data(G_OBJECT(userdata), "dms_ui_data");
  /*Fill/clear tokens box*/
  if(gtk_toggle_button_get_active(btn)){
    /*Add tokens to box*/
      /*Switch tokens btn to FALSE*/
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dms_ui->tokens_btn), FALSE);
    if(gstate->mnum == 0){
      return;
    }
    for(int32_t i = 0; i < gstate->mnum; ++i){
      /*Create widget*/
      GdkTexture* texture = gdk_texture_new_from_filename(gstate->maps[i].cache, nullptr);
      GtkWidget* image = gtk_image_new_from_paintable(GDK_PAINTABLE(texture));
      tokensdata* tdata = (tokensdata*)g_new0(tokensdata, 1);
      if(!tdata){
        continue;
      }
      tdata->id = i;
      tdata->type = MAP;
      /*Add tokens image to box*/
      gtk_box_append(GTK_BOX(dms_ui->tokens_box), image);
      /*Drag and drop prepare settings*/
      GtkDragSource* drag_source = gtk_drag_source_new();
      g_signal_connect_data(drag_source, "prepare", G_CALLBACK(on_token_drag_prepare),
                            (gpointer)tdata, (GClosureNotify)(g_free), 0);
      g_signal_connect(drag_source, "drag-begin", G_CALLBACK(on_token_drag_begin), image);
      /*Set dnd controller*/
      gtk_widget_add_controller(image, GTK_EVENT_CONTROLLER(drag_source));
      /*Free*/
      g_object_unref(texture);
    }
    gtk_widget_set_visible(dms_ui->tokens_box, TRUE);
  }else{
    /*Clear box*/
    GtkWidget* child;
    while((child = gtk_widget_get_first_child(dms_ui->tokens_box))){
      gtk_box_remove(GTK_BOX(dms_ui->tokens_box), child);
    }
    gtk_widget_set_visible(dms_ui->tokens_box, FALSE);
  }
}

