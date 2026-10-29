#include "EventsSystem/file_dnd.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_resize2.h>
#include <stb_image_write.h>

#include "GameState/game_state.h"
#include "GameState/file_initialize.h"
#include "Map/map_data.h"
#include "Map/map.h"
#include "Map/map_renderer.h"
#include "Tokens/tokens.h"
#include "Map/texture_atlas.h"
#include "Map/texture.h"

typedef struct{
  GdkFileList* filelist;
  gpointer userdata;
  int32_t toktype;
}drop_task_data;

typedef struct{
  gpointer userdata;
  int32_t type;
  int32_t id;
}foratlas;

typedef struct{
  struct layer* l;
  struct renderer* ren;
  char* atlas_path;
}forlayer;

static void remove_drag_overlay(GtkDropTargetAsync* target);

static void work_with_files(GTask* task, gpointer source, gpointer taskdata, GCancellable* cancelablle);
static void on_value_read_ready_toks(GObject* source, GAsyncResult* res, gpointer userdata);
static void on_value_read_ready_maps(GObject* source, GAsyncResult* res, gpointer userdata);
static void free_drop_task_data(gpointer data);
static void linear_resize_img(GFile* img, int32_t toktype);

static void on_mime_type_read_toks_done(GObject* source, GAsyncResult* res, gpointer userdata);
static void on_stream_read_done(GObject* sourcestream, GAsyncResult* res, gpointer userdata);

static void work_with_tokens(GTask* task, gpointer source, gpointer taskdata, GCancellable* cancelablle);
static void set_layer_texture_atlas(GObject* source, GAsyncResult* res, gpointer userdata);

gboolean on_toks_drag_enter(GtkDropTargetAsync* target, GdkDrop* drop, double x, double y, gpointer userdata)
{
  GdkContentFormats* formats = gdk_drop_get_formats(drop);
  if(formats && gdk_content_formats_contain_mime_type(formats, "text/plain")){
    return TRUE;
  }
  GtkWidget* overlay = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(target));
  /*Add labels to the overlay*/
  GtkWidget* mainbox = gtk_center_box_new();
  gtk_widget_set_halign(mainbox, GTK_ALIGN_CENTER);
  GtkWidget* toksbox = gtk_center_box_new();
  GtkWidget* mapsbox = gtk_center_box_new();
  GtkWidget* tokslabel = GTK_WIDGET(gtk_label_new("TOKENS"));
  GtkWidget* mapslabel = GTK_WIDGET(gtk_label_new("MAPS"));
  gtk_center_box_set_center_widget(GTK_CENTER_BOX(toksbox), tokslabel);
  gtk_center_box_set_center_widget(GTK_CENTER_BOX(mapsbox), mapslabel);
  gtk_center_box_set_start_widget(GTK_CENTER_BOX(mainbox), GTK_WIDGET(mapsbox));
  gtk_center_box_set_end_widget(GTK_CENTER_BOX(mainbox), GTK_WIDGET(toksbox));
  gtk_overlay_add_overlay(GTK_OVERLAY(overlay), mainbox);
  g_object_set_data(G_OBJECT(target), "drag_overlay_widget", mainbox);
  return TRUE;
}

void on_toks_drag_leave(GtkDropTargetAsync* target, gpointer userdata)
{
  remove_drag_overlay(target);
}

gboolean on_toks_drop(GtkDropTargetAsync* target, GdkDrop* drop,
                       double x, double y, gpointer userdata)
{
  GdkContentFormats* formats = gdk_drop_get_formats(drop);
  /*Get content provider*/
  GdkDrag* drag = gdk_drop_get_drag(drop);
  if(formats && gdk_content_formats_contain_mime_type(formats, "text/plain")){
    g_object_ref(drop);
    g_object_set_data(G_OBJECT(userdata), "drop", drop);
    const char* mimes[] = {"text/plain", nullptr};
    gdk_drop_read_async(drop, mimes,
                        G_PRIORITY_DEFAULT, nullptr,
                        on_mime_type_read_toks_done, userdata);
    return TRUE;
  }
  remove_drag_overlay(target);
  /*Get window width*/
  GtkWidget* overlay = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(target));
  GdkSurface* surface = gtk_native_get_surface(gtk_widget_get_native(overlay));
  double width = 0;
  if(surface){
    width = (double)gdk_surface_get_width(surface);
  }
  if(x >= (width / 2.0)){
    gdk_drop_read_value_async(drop, GDK_TYPE_FILE_LIST, G_PRIORITY_DEFAULT,
                            nullptr, on_value_read_ready_toks, userdata);
  }else{
    gdk_drop_read_value_async(drop, GDK_TYPE_FILE_LIST, G_PRIORITY_DEFAULT,
                            nullptr, on_value_read_ready_maps, userdata);
  }
  return TRUE;
}

static void remove_drag_overlay(GtkDropTargetAsync* target)
{
  GtkWidget* mainbox = GTK_WIDGET(g_object_get_data(G_OBJECT(target), "drag_overlay_widget"));
  if(mainbox){
    GtkWidget* overlay = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(target));
    gtk_overlay_remove_overlay(GTK_OVERLAY(overlay), mainbox);
    g_object_set_data(G_OBJECT(target), "drag_overlay_widget", nullptr);
  }
}

static void work_with_files(GTask* task, gpointer source, gpointer taskdata, GCancellable* cancelablle)
{
  drop_task_data* data = (drop_task_data*)taskdata;
  GdkDrop* drop = GDK_DROP(source);
  GSList* files = gdk_file_list_get_files(data->filelist);
  /*Copy files into dir*/
    /*Create paths*/
  struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(data->userdata), "gstate");
  const char* saves = "saves";
  const char* dest = "presets/textures";
  char dest_dir_path[256];
  if(data->toktype == TOKEN){
    snprintf(dest_dir_path, sizeof(dest_dir_path), "%s/%s/%s/tokens", saves, gstate->gamename, dest);
  }else{
    snprintf(dest_dir_path, sizeof(dest_dir_path), "%s/%s/%s/maps", saves, gstate->gamename, dest);
  }
    /*Copy*/
  GFile* destdir = g_file_new_for_path(dest_dir_path);
  for(GSList* l = files; l != nullptr; l = l->next){
    GFile* sourcefile = G_FILE(l->data);
    char* basename = g_file_get_basename(sourcefile);
    size_t len = strlen(basename);
    if(basename[len - 4] != '.' || !(basename[len - 3] == 'j' || basename[len - 3] == 'p')){
      g_free(basename);
      continue;
    }
    GFile* destfile = g_file_get_child(destdir, basename);
    g_file_copy(sourcefile, destfile, G_FILE_COPY_OVERWRITE, nullptr, nullptr, nullptr, nullptr);
    linear_resize_img(destfile, data->toktype);
    /*Gstate register token*/
    char* path_to_img = g_file_get_path(destfile);
    game_state_register_token(gstate, data->toktype, path_to_img);
    /*Free*/
    g_free(path_to_img);
    g_free(basename);
    g_object_unref(destfile);
  }
  /*Free*/
  g_slist_free(files);
  g_object_unref(destdir);
  gdk_drop_finish(drop, GDK_ACTION_COPY);
}

static void on_value_read_ready_toks(GObject* source, GAsyncResult* res, gpointer userdata)
{
  GdkDrop* drop = GDK_DROP(source);
  const GValue* value = gdk_drop_read_value_finish(drop, res, nullptr);
  /*To drop_task_data*/
  drop_task_data* data = g_new0(drop_task_data, 1);
  data->filelist = g_value_dup_boxed(value);
  data->userdata = userdata;
  data->toktype = TOKEN;
  /*New THREAD*/
  GTask* task = g_task_new(drop, nullptr, nullptr, nullptr);
  g_task_set_task_data(task, data, free_drop_task_data);
  g_task_run_in_thread(task, work_with_files);
  /*Free*/
  g_object_unref(task);
}

static void on_value_read_ready_maps(GObject* source, GAsyncResult* res, gpointer userdata)
{
  GdkDrop* drop = GDK_DROP(source);
  const GValue* value = gdk_drop_read_value_finish(drop, res, nullptr);
  /*To drop_task_data*/
  drop_task_data* data = g_new0(drop_task_data, 1);
  data->filelist = g_value_dup_boxed(value);
  data->userdata = userdata;
  data->toktype = MAP;
  /*New THREAD*/
  GTask* task = g_task_new(drop, nullptr, nullptr, nullptr);
  g_task_set_task_data(task, data, free_drop_task_data);
  g_task_run_in_thread(task, work_with_files);
  /*Free*/
  g_object_unref(task);
}

static void free_drop_task_data(gpointer data)
{
  drop_task_data* subdata = (drop_task_data*)data;
  g_boxed_free(GDK_TYPE_FILE_LIST, subdata->filelist);
  g_free(subdata);
}

static void linear_resize_img(GFile* img, int32_t toktype)
{
  int percentage = 100 - 50 * toktype;
  /*Get file path*/
  char* file = g_file_get_path(img);
  char* filename = g_file_get_basename(img);
  int file_name_length = strlen(filename);
  /*Get file*/
  stbi_set_flip_vertically_on_load(false);
  int w, h, ch;
  uint8_t* i = stbi_load(file, &w, &h, &ch, 0);
  if(strlen(filename) < 4 || !i){
    goto half_free;
  }
  /*Output size*/
  int outw = (w * percentage) / 100;
  int outh = (h * percentage) / 100;
  /*Output image*/
  uint8_t* outi = calloc(1, (size_t)outw * outh * ch * sizeof(uint8_t));
  /*Output stride*/
  int inputstride = w * ch;
  int outputstride = outw * ch;
  stbir_pixel_layout layout = (ch == 4) ? STBIR_RGBA : STBIR_RGBA;
  if(filename[file_name_length - 3] == 'p'){
    stbir_resize_uint8_linear(i, w, h, inputstride, outi, outw, outh, outputstride, layout);
    stbi_write_png(file, outw, outh, ch, outi, outputstride);
  }else{
    stbir_resize_uint8_linear(i, w, h, inputstride, outi, outw, outh, outputstride, layout);
    stbi_write_jpg(file, outw, outh, ch, outi, 100);
  }
  /*Free*/
  stbi_image_free(outi);
  stbi_image_free(i);
half_free:
  g_free(filename);
  g_free(file);
}

static void on_mime_type_read_toks_done(GObject* source, GAsyncResult* res, gpointer userdata)
{
  GdkDrop* drop = GDK_DROP(source);
  GError* err = nullptr;
  const char* out_mime = nullptr;
  GInputStream* stream = gdk_drop_read_finish(drop, res, &out_mime, &err);
  if(!stream){
    g_warning("Read error: %s\n", err ? err->message : "unknown");
    g_clear_error(&err);
    gdk_drop_finish(drop, 0);
    g_object_unref(drop);
    return;
  }
  char* buffer = g_malloc0(64);
  g_object_set_data_full(G_OBJECT(stream), "buffer", buffer, g_free);
  g_object_set_data(G_OBJECT(userdata), "drop_mime", drop);
  g_input_stream_read_all_async(stream, buffer, 63, G_PRIORITY_DEFAULT,
                                nullptr, on_stream_read_done, userdata);
  g_object_unref(stream);
}

static void on_stream_read_done(GObject* sourcestream, GAsyncResult* res, gpointer userdata)
{
  GInputStream* stream = G_INPUT_STREAM(sourcestream);
  GdkDrop* drop = GDK_DROP(g_object_get_data(G_OBJECT(userdata), "drop_mime"));
  gsize bytesread = 0;
  GError* err = nullptr;
  /*Read*/
  if(!g_input_stream_read_all_finish(stream, res, &bytesread, &err)){
    g_warning("Read error: %s\n", err ? err->message : "unknown");
    g_clear_error(&err);
    gdk_drop_finish(drop, 0);
    g_object_unref(drop);
    return;
  }
  char* type_id = (char*)g_object_get_data(G_OBJECT(stream), "buffer");
  type_id[bytesread] = '\0';
  gdk_drop_finish(drop, GDK_ACTION_COPY);
  g_object_unref(drop);
  /* Parsing "<type>-<id>"*/
  char* _id = nullptr;
  int32_t type = strtol(type_id, &_id, 10);
  int32_t id = 0;
  if(_id && *_id) sscanf(&_id[1], "%d", &id);
  /*Set data for task*/
  foratlas* data = g_new0(foratlas, 1);
  data->userdata = userdata;
  data->type = type;
  data->id = id;
  /*Create new task*/
  GTask* task = g_task_new(nullptr, nullptr, set_layer_texture_atlas, data);
  g_task_set_task_data(task, data, nullptr);
  g_task_run_in_thread(task, work_with_tokens);
  g_object_unref(task);
}

static void work_with_tokens(GTask* task, gpointer source, gpointer taskdata, GCancellable* cancelablle)
{
  foratlas* data = (foratlas*)taskdata;
  forlayer* fl = malloc(sizeof(forlayer));
  fl->ren = nullptr;
  fl->l = nullptr;
  /*Get game state*/
  struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(data->userdata), "gstate");
  /*Get current game scene*/
  struct gamescene* current = &gstate->gscns[gstate->current_map_id - 1];
  struct layer* l = nullptr;
  /*Set right layer*/
  int32_t lrnum = -1;
  for(;;){
    for(int32_t i = 0; i < gstate->m->lrnum; ++i){
      if(gstate->m->l[i].toktype == data->type && gstate->m->l[i].type == IMG){
        lrnum = i;
        break;
      }
    }
    if(lrnum >= 0){
      break;
    }
    g_print("Layer with type %d not exist\n Creating new layer!\n", data->type);
    if(!map_add_layer(gstate->m, IMG, data->type)){
      /*Free and return*/
      free(fl);
      g_task_return_pointer(task, nullptr, nullptr);
      return;
    }
  }
  l = &gstate->m->l[lrnum];
    /*Get right tokens*/
  char* atlas_path = nullptr;
    /*Define atlas_dir_path*/
  char* atlas_dir_path = (char*)calloc(512, sizeof(char));
  snprintf(atlas_dir_path, 512, "saves/%s/presets/atlas/%d", gstate->gamename, gstate->current_map_id);
    /*Create atlas dir*/
  makedir(atlas_dir_path);
    /*Made atlas*/
  if(data->type == TOKEN && l->toktype == TOKEN){
    struct token* ts = (struct token*)malloc((size_t)(l->rnum) * sizeof(struct token));
    for(int32_t i = 0; i < l->rnum; ++i){
      for(int32_t j = 0; j < gstate->tnum; ++j){
        if(l->tokid[i] == gstate->tokens[j].id){
          ts[i] = gstate->tokens[j];
        }
      }
    }
      /*Create atlas*/
    float w, h;
    get_path_to_atlas(atlas_dir_path, lrnum, ts, l->rnum, &w, &h, &atlas_path);
    /*Get token*/
    struct token* t = &gstate->tokens[data->id];
    /*Put token into layer*/
    struct rectangle rect;
    rect_set_XYWH(&rect, current->cam.cam_x, current->cam.cam_y,
                  current->l[0].gridstep, current->l[0].gridstep);
    rect_set_RGBA(&rect, 1.0f, 1.0f, 1.0f, 1.0f);
    rect_set_UV(&rect, t->u0, t->v0, t->u1, t->v1);
    layer_add_rect(l, &rect);
    layer_link_last_rect_with_token(l, t);
    /*Free*/
    free(ts);
    free(atlas_dir_path);
  }else if(data->type == MAP && l->toktype == MAP){
    struct token* ms = (struct token*)malloc((size_t)(l->rnum) * sizeof(struct token));
    for(int32_t i = 0; i < l->rnum; ++i){
      for(int32_t j = 0; j < gstate->tnum; ++j){
        if(l->tokid[i] == gstate->tokens[j].id){
          ms[i] = gstate->tokens[j];
        }
      }
    }
      /*Create atlas*/
    float w, h;
    get_path_to_atlas(atlas_dir_path, lrnum, ms, l->rnum, &w, &h, &atlas_path);
    /*Get map*/
    struct token* m = &gstate->maps[data->id];
    /*Put token into layer*/
    struct rectangle rect;
    rect_set_XYWH(&rect, current->cam.cam_x, current->cam.cam_y, w, h);
    rect_set_RGBA(&rect, 1.0f, 1.0f, 1.0f, 1.0f);
    rect_set_UV(&rect, m->u0, m->v0, m->u1, m->v1);
    layer_add_rect(l, &rect);
    layer_link_last_rect_with_token(l, m);
    /*Free*/
    free(ms);
    free(atlas_dir_path);
  }else{
    g_error("ATLAS CAN'T BE CREATED, WRONG TOKENS TYPE!!!\n");
    free(atlas_dir_path);
    return;
  }
  /*Set for layer data*/
  fl->l = l;
  fl->ren = &gstate->m->ren[lrnum];
  fl->atlas_path = atlas_path;
  g_task_return_pointer(task, fl, free);
}

static void set_layer_texture_atlas(GObject* source, GAsyncResult* res, gpointer userdata)
{
  GTask* task = G_TASK(res);
  foratlas* data = (foratlas*)userdata;
  GError* err = nullptr;
  /*Get atlas path*/
  forlayer* fl = g_task_propagate_pointer(task, &err);
  if(err){
    g_warning("Something gone wrong, atlas path not recived: %s\n", err->message);
    g_clear_error(&err);
    g_error_free(err);
    g_free(data);
    free(fl);
    return;
  }
  /*Make current gl context*/
  GtkGLArea* gl_area = GTK_GL_AREA(g_object_get_data(G_OBJECT(data->userdata), "gl_area"));
  gtk_gl_area_make_current(gl_area); //Maybe need check errors?
  /*Create new renderer*/
  createrenderer(fl->ren, fl->l, IMG);
  /*Layer update texture atlas*/
  layer_update_texture_atlas(fl->l, fl->atlas_path);
  /*Free*/
  g_free(data);
  free(fl->atlas_path);
}
