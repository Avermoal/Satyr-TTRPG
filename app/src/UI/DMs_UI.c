#include "UI/DMs_UI.h"

#include <stdint.h>
#include <string.h>

#include "UI/UIs_info.h"
#include "GameState/game_state.h"
#include "Map/map_data.h"
#include "Tokens/tokens.h"
#include "EventsSystem/token_dnd.h"
#include "DisplaySystem/display_system.h"
#include "EventsSystem/event_system.h"
#include "EventsSystem/file_dnd.h"

typedef struct{
  GtkToggleButton* togglebtn;
  GtkWidget* box;
  struct gamescene* gscn;
  int32_t lid;
  int32_t tid;
}data_to_delete;

static void on_side_menu_btn_clicked(GtkButton* btn, gpointer userdata);
static void on_side_hide_btn_clicked(GtkButton* btn, gpointer userdata);

/*Top menu*/
static void on_tokens_btn_toggled(GtkToggleButton* btn, gpointer userdata);
static void on_maps_btn_toggled(GtkToggleButton* btn, gpointer userdata);
static gboolean on_tokens_scrolled_win_scroll_block(GtkEventControllerScroll* block, double dx, double dy, gpointer userdata);
static void on_settings_button_toggled(GtkToggleButton* btn, gpointer userdata);
static void on_grid_scale_value_changed(GtkRange* range, gpointer useradta);
static void on_grid_step_value_changed(GtkRange* range, gpointer useradta);
static void on_delete_token_from_scene_clicked(GtkButton* btn, gpointer userdata);

void create_DMs_UI(int ISNEW, gpointer userdata)
{
  GtkWindow* win = GTK_WINDOW(userdata);
  /*CREATE dms_page UI*/
    /*Allocation memory*/
  struct DMs_UI* dms_ui = (struct DMs_UI*)calloc(1, sizeof(struct DMs_UI));
    /*DMs*/
  GError* err = nullptr;
  GtkBuilder* dms_builder = gtk_builder_new();
  gtk_builder_add_from_file(dms_builder, DMS_SOURCES, &err);
  if(err){
    g_print("DMs page builder error: %s\n", err->message);
    g_object_unref(dms_builder);
    g_error_free(err);
    return;
  }
  /*Set DMs UI widgets*/
  dms_ui->side_menu_revealer = GTK_REVEALER(gtk_builder_get_object(dms_builder, "side_menu_revealer"));
  dms_ui->side_box_for_btn = GTK_BOX(gtk_builder_get_object(dms_builder, "side_box_for_btn"));
  dms_ui->side_menu_btn = GTK_BUTTON(gtk_builder_get_object(dms_builder, "side_menu_btn"));
  dms_ui->overlay = GTK_WIDGET(gtk_builder_get_object(dms_builder, "dms_page"));
  dms_ui->tokens_box = GTK_WIDGET(gtk_builder_get_object(dms_builder, "tokens_box"));
  dms_ui->tokens_btn = GTK_WIDGET(gtk_builder_get_object(dms_builder, "tokens_btn"));
  dms_ui->maps_btn = GTK_WIDGET(gtk_builder_get_object(dms_builder, "maps_btn"));
  dms_ui->drag_btn = GTK_WIDGET(gtk_builder_get_object(dms_builder, "drag_btn"));
  dms_ui->settings_btn = GTK_WIDGET(gtk_builder_get_object(dms_builder, "settings_btn"));
  dms_ui->settings_grid = GTK_WIDGET(gtk_builder_get_object(dms_builder, "settings_grid"));
  dms_ui->layers_box = GTK_WIDGET(gtk_builder_get_object(dms_builder, "layers_box"));
  g_object_set_data(G_OBJECT(userdata), "dms_ui_data", dms_ui);
  g_object_set_data(G_OBJECT(dms_ui->overlay), "drag_btn", dms_ui->drag_btn);
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
  g_signal_connect(dms_ui->tokens_btn, "toggled", G_CALLBACK(on_tokens_btn_toggled), userdata);
  g_signal_connect(dms_ui->maps_btn, "toggled", G_CALLBACK(on_maps_btn_toggled), userdata);
  GtkGesture* longpress = gtk_gesture_drag_new();
  g_signal_connect(longpress, "drag-begin", G_CALLBACK(on_drag_begin), userdata);
  g_signal_connect(longpress, "drag-end", G_CALLBACK(on_drag_end), userdata);
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(longpress), 0);
  gtk_widget_add_controller(dms_ui->overlay, GTK_EVENT_CONTROLLER(longpress));
  g_signal_connect(dms_ui->settings_btn, "toggled", G_CALLBACK(on_settings_button_toggled), userdata);
  /*Set drag btn active*/
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dms_ui->drag_btn), TRUE);
  /*Set Drag and Drop target*/
    /*Content formats*/
  GdkContentFormatsBuilder* formatsbuilder = gdk_content_formats_builder_new();
  gdk_content_formats_builder_add_mime_type(formatsbuilder, "text/uri-list");
  GdkContentFormats* formats = gdk_content_formats_builder_free_to_formats(formatsbuilder);
    /*Set*/
  GtkDropTargetAsync* target = gtk_drop_target_async_new(
    gdk_content_formats_new((const char* []){"text/uri-list", "text/plain"}, 2), GDK_ACTION_COPY);
  g_signal_connect(target, "drop", G_CALLBACK(on_toks_drop), userdata);
  g_signal_connect(target, "drag-enter", G_CALLBACK(on_toks_drag_enter), userdata);
  g_signal_connect(target, "drag-leave", G_CALLBACK(on_toks_drag_leave), userdata);
  gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(target), GTK_PHASE_CAPTURE);
  gtk_widget_add_controller(dms_ui->overlay, GTK_EVENT_CONTROLLER(target));
    /*Free formats*/
  gdk_content_formats_unref(formats);
  /*Set scroll controller*/
  GtkEventController* scroll = gtk_event_controller_scroll_new(
    GTK_EVENT_CONTROLLER_SCROLL_VERTICAL
  );
  g_signal_connect(scroll, "scroll", G_CALLBACK(onscroll), userdata);
  gtk_widget_add_controller(dms_ui->overlay, GTK_EVENT_CONTROLLER(scroll));
  /*Scroll intercept for tokens_scrolled_win*/
  GtkWidget* tokens_scrolled_win = GTK_WIDGET(gtk_builder_get_object(dms_builder, "tokens_scrolled_win"));
  GtkEventController* blockscroll = gtk_event_controller_scroll_new(
    GTK_EVENT_CONTROLLER_SCROLL_VERTICAL
  );
  g_signal_connect(blockscroll, "scroll", G_CALLBACK(on_tokens_scrolled_win_scroll_block), nullptr);
  gtk_widget_add_controller(tokens_scrolled_win, GTK_EVENT_CONTROLLER(blockscroll));
  /*Motion controller*/
  GtkEventController* motion = gtk_event_controller_motion_new();
  g_signal_connect(motion, "motion", G_CALLBACK(onmotion), userdata);
  gtk_widget_add_controller(dms_ui->overlay, motion);
  /*IS NEW GAME*/
  g_object_set_data(G_OBJECT(userdata), "ISNEW", GINT_TO_POINTER(ISNEW));
  /*Switch to dms_page*/
  GtkStack* interfacestack = g_object_get_data(G_OBJECT(userdata), "interfacestack");
  GtkWidget* dms_page = GTK_WIDGET(gtk_builder_get_object(dms_builder, "dms_page"));
  gtk_stack_add_named(interfacestack, dms_page, "dms_page");
  gtk_stack_set_visible_child_name(interfacestack, "dms_page");
  /*Unref builder*/
  g_object_unref(dms_builder);}

/*Side menu btns*/
static void on_side_menu_btn_clicked(GtkButton* btn, gpointer userdata)
{
  gtk_widget_set_visible(GTK_WIDGET(btn), false);

  struct DMs_UI* dms_ui = (struct DMs_UI*)g_object_get_data(G_OBJECT(userdata), "dms_ui_data");
  gtk_revealer_set_reveal_child(dms_ui->side_menu_revealer, true);
  gtk_widget_set_can_target(GTK_WIDGET(dms_ui->side_box_for_btn), false);
}

static void on_side_hide_btn_clicked(GtkButton* btn, gpointer userdata)
{
  struct DMs_UI* dms_ui = (struct DMs_UI*)g_object_get_data(G_OBJECT(userdata), "dms_ui_data");
  gtk_revealer_set_reveal_child(dms_ui->side_menu_revealer, false);
  gtk_widget_set_can_target(GTK_WIDGET(dms_ui->side_box_for_btn), true);
  gtk_widget_set_visible(GTK_WIDGET(dms_ui->side_menu_btn), true);
}

/*Top menu*/
static void on_tokens_btn_toggled(GtkToggleButton* btn, gpointer userdata)
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
      gtk_image_set_pixel_size(GTK_IMAGE(image), 64);
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

static void on_maps_btn_toggled(GtkToggleButton* btn, gpointer userdata)
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
      gtk_image_set_pixel_size(GTK_IMAGE(image), 64);
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

static gboolean on_tokens_scrolled_win_scroll_block(GtkEventControllerScroll* block, double dx, double dy, gpointer userdata)
{
  /*Only stop*/
  return GDK_EVENT_STOP;
}

/*Left settings*/
static void on_settings_button_toggled(GtkToggleButton* btn, gpointer userdata)
{
  /*Get dms ui data*/
  struct DMs_UI* dms_ui = (struct DMs_UI*)g_object_get_data(G_OBJECT(userdata), "dms_ui_data");
  /*Get gstate*/
  struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(userdata), "gstate");
  /*Toggle*/
  GtkBox* layers_box = GTK_BOX(dms_ui->layers_box);
  if(gtk_toggle_button_get_active(btn)){
    /*Layers*/
    struct gamescene* current = &gstate->gscns[gstate->current_map_id - 1];
    for(int32_t i = 0; i < current->lrnum; ++i){
      if(current->l[i].type == GRID){
        GtkWidget* gridlabel = gtk_label_new("Grid");
        GtkWidget* scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 1.0, 0.01);
        gtk_range_set_value(GTK_RANGE(scale), current->l[i].gridopacity);
        gtk_scale_set_draw_value(GTK_SCALE(scale), TRUE);
        gtk_widget_set_size_request(scale, 150, -1);
        gtk_widget_set_halign(scale, GTK_ALIGN_START);
        gtk_scale_set_digits(GTK_SCALE(scale), 2);
        gtk_widget_set_hexpand(scale, TRUE);
        GtkWidget* step = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 4.0, 124.0, 0.1);
        gtk_range_set_value(GTK_RANGE(step), current->l[i].gridstep);
        gtk_scale_set_draw_value(GTK_SCALE(step), TRUE);
        gtk_widget_set_size_request(step, 150, -1);
        gtk_widget_set_halign(step, GTK_ALIGN_START);
        gtk_scale_set_digits(GTK_SCALE(step), 0);
        gtk_widget_set_hexpand(step, TRUE);
        /*Grid opacity signal*/
        g_signal_connect(GTK_RANGE(scale), "value-changed", G_CALLBACK(on_grid_scale_value_changed), current);
        /*Grid step siganl*/
        g_signal_connect(GTK_RANGE(step), "value-changed", G_CALLBACK(on_grid_step_value_changed), current);
        gtk_box_append(layers_box, gridlabel);
        gtk_box_append(layers_box, scale);
        gtk_box_append(layers_box, step);
      }else if(current->l[i].type == IMG){
        if(current->l[i].toktype == MAP){
          GtkWidget* maplabel = gtk_label_new("Maps:");
          gtk_box_append(layers_box, maplabel);
          for(int32_t j = 0; j < current->l[i].rnum; ++j){
            GtkWidget* box = nullptr;
            GtkWidget* label = nullptr;
            GtkWidget* delbtn = nullptr;
            for(int32_t k = 0; k < gstate->mnum; ++k){
              if(current->l[i].tokid[j] == gstate->maps[k].id){
                data_to_delete* data = (data_to_delete*)calloc(1, sizeof(data_to_delete));
                if(!data){
                  continue;
                }
                box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
                label = gtk_label_new(get_basename(gstate->maps[k].path));
                delbtn = gtk_button_new_from_icon_name("list-remove");
                gtk_box_append(GTK_BOX(box), label);
                gtk_box_append(GTK_BOX(box), delbtn);
                data->togglebtn = btn;
                data->box = box;
                data->gscn = current;
                data->lid = i;
                data->tid = j;
                /*Signal to delete token from scene*/
                g_signal_connect_data(delbtn,
                                      "clicked",
                                      G_CALLBACK(on_delete_token_from_scene_clicked),
                                      data,
                                      (GClosureNotify)free,
                                      G_CONNECT_AFTER);
                gtk_box_append(layers_box, box);
              }
            }
          }
        }else if(current->l[i].toktype == TOKEN){
          GtkWidget* maplabel = gtk_label_new("Tokens:");
          gtk_box_append(layers_box, maplabel);
          for(int32_t j = 0; j < current->l[i].rnum; ++j){
            GtkWidget* box = nullptr;
            GtkWidget* label = nullptr;
            GtkWidget* delbtn = nullptr;
            for(int32_t k = 0; k < gstate->tnum; ++k){
              if(current->l[i].tokid[j] == gstate->tokens[k].id){
                data_to_delete* data = (data_to_delete*)calloc(1, sizeof(data_to_delete));
                if(!data){
                  continue;
                }
                box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
                label = gtk_label_new(get_basename(gstate->tokens[k].path));
                delbtn = gtk_button_new_from_icon_name("list-remove");
                gtk_box_append(GTK_BOX(box), label);
                gtk_box_append(GTK_BOX(box), delbtn);
                data->togglebtn = btn;
                data->box = box;
                data->gscn = current;
                data->lid = i;
                data->tid = j;
                /*Signal to delete token from scene*/
                g_signal_connect_data(delbtn,
                                      "clicked",
                                      G_CALLBACK(on_delete_token_from_scene_clicked),
                                      data,
                                      (GClosureNotify)free,
                                      G_CONNECT_AFTER);
                gtk_box_append(layers_box, box);
              }
            }
          }
        }
      }
    }
  }else{
    GtkWidget* child = nullptr;
    while((child = gtk_widget_get_first_child(GTK_WIDGET(layers_box))) != nullptr){
      gtk_box_remove(layers_box, child);
    }
  }
}

static void on_grid_scale_value_changed(GtkRange* range, gpointer userdata)
{
  /*Get current scene grid (zero)*/
  struct gamescene* current = (struct gamescene*)userdata;
  if(!current){
    return;
  }
  struct layer* gridlayer = &current->l[0];
  if(gridlayer->type != GRID){
    return;
  }
  gridlayer->gridopacity = (float)gtk_range_get_value(range);
}

static void on_grid_step_value_changed(GtkRange* range, gpointer userdata)
{
  /*Get current scene grid (zero)*/
  struct gamescene* current = (struct gamescene*)userdata;
  if(!current){
    return;
  }
  struct layer* gridlayer = &current->l[0];
  if(gridlayer->type != GRID){
    return;
  }
  gridlayer->gridstep = (float)gtk_range_get_value(range);
}

static void on_delete_token_from_scene_clicked(GtkButton* btn, gpointer userdata)
{
  data_to_delete* data = (data_to_delete*)userdata;
  /*Delete token from layer*/
  if(layer_remove_rt_by_id(&data->gscn->l[data->lid], data->tid)){
    gtk_toggle_button_set_active(data->togglebtn, FALSE);
    /*Unparent widget*/
    gtk_widget_unparent(data->box);
    gtk_toggle_button_set_active(data->togglebtn, TRUE);
  }
}
