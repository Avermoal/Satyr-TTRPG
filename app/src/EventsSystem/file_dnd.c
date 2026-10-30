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
  char** path;
  int32_t pnum;
  int32_t toktype;
}drop_task_data_final;

typedef struct{
  gpointer userdata;
  struct token* righttoks;
  int32_t lrnum;
  int32_t tnum;
  bool is_new_layer;
}foratlas;

typedef struct{
  char* atlas_path;
  struct token* not_link_tok;
  float w, h;
}forlayer;

static void remove_drag_overlay(GtkDropTargetAsync* target);

static void work_with_files(GTask* task, gpointer source, gpointer taskdata, GCancellable* cancelablle);
static void drop_final_data_free(gpointer data);
static void on_value_read_ready_toks(GObject* source, GAsyncResult* res, gpointer userdata);
static void on_value_read_ready_maps(GObject* source, GAsyncResult* res, gpointer userdata);
static void on_work_with_files_completed(GObject* source, GAsyncResult* res, gpointer userdata);
static void free_drop_task_data(gpointer data);
static void linear_resize_img(GFile* img, int32_t toktype);

static void on_mime_type_read_toks_done(GObject* source, GAsyncResult* res, gpointer userdata);
static void on_stream_read_done(GObject* sourcestream, GAsyncResult* res, gpointer userdata);

static void work_with_atlas(GTask* task, gpointer source, gpointer taskdata, GCancellable* cancelablle);
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
  drop_task_data_final* datafinal = (drop_task_data_final*)malloc(sizeof(drop_task_data_final));
  datafinal->userdata = data->userdata;
  datafinal->path = (char**)calloc(g_slist_length(files), sizeof(char*));
  datafinal->toktype = data->toktype;
  datafinal->pnum = 0;
  for(GSList* l = files; l != nullptr; l = l->next){
    GFile* sourcefile = G_FILE(l->data);
    char* basename = g_file_get_basename(sourcefile);
    size_t len = strlen(basename);
    if(basename[len - 4] != '.' || !(basename[len - 3] == 'j' || basename[len - 3] == 'p')){
      datafinal->path[datafinal->pnum++] = nullptr;
      g_free(basename);
      continue;
    }
    GFile* destfile = g_file_get_child(destdir, basename);
    g_file_copy(sourcefile, destfile, G_FILE_COPY_OVERWRITE, nullptr, nullptr, nullptr, nullptr);
    linear_resize_img(destfile, data->toktype);
    /*Set return pointer*/
    datafinal->path[datafinal->pnum++] = g_file_get_path(destfile);
    /*Free*/
    g_free(basename);
    g_object_unref(destfile);
  }
  g_task_return_pointer(task, datafinal, drop_final_data_free);
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
  GTask* task = g_task_new(drop, nullptr, on_work_with_files_completed, nullptr);
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
  GTask* task = g_task_new(drop, nullptr, on_work_with_files_completed, nullptr);
  g_task_set_task_data(task, data, free_drop_task_data);
  g_task_run_in_thread(task, work_with_files);
  /*Free*/
  g_object_unref(task);
}

static void on_work_with_files_completed(GObject* source, GAsyncResult* res, gpointer userdata)
{
  GTask* task = G_TASK(res);
  GError* err = nullptr;
  /*Read pointer*/
  drop_task_data_final* data = (drop_task_data_final*)g_task_propagate_pointer(task, &err);
  if(err){
    g_warning("Final files drop data not readed: %s\n", err->message);
    g_error_free(err);
    drop_final_data_free(data);
    return;
  }
  if(data){
    /*Get game state*/
    struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(data->userdata), "gstate");
    /*Register tokens in main thread*/
    for(int32_t i = 0; i < data->pnum; ++i){
      if(data->path[i]){
        game_state_register_token(gstate, data->toktype, data->path[i]);
      }
    }
    /*Free*/
    drop_final_data_free(data);
  }
}

static void drop_final_data_free(gpointer data)
{
  drop_task_data_final* datafinal = (drop_task_data_final*)data;
  for(int32_t i = 0; i < datafinal->pnum; ++i){
    /*path[i] can be nullptr*/
    free(datafinal->path[i]);
    datafinal->path[i] = nullptr;
  }
  free(datafinal->path);
  datafinal->path = nullptr;
  free(datafinal);
  datafinal = nullptr;
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
  stbir_pixel_layout layout = (ch == 4) ? STBIR_RGBA : STBIR_RGB;
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

/*-------------------TOKENS WORK ----------------------*/

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
  /*Free and drop finish*/
  gdk_drop_finish(drop, GDK_ACTION_COPY);
  g_object_unref(drop);
  /*Parsing "<type>-<id>"*/
  char* _id = nullptr;
  int32_t type = strtol(type_id, &_id, 10);
  int32_t id = 0;
  if(_id && *_id){
    sscanf(&_id[1], "%d", &id);
  }
  /*Allocate memory for foratlas data*/
  foratlas* data = g_new0(foratlas, 1);
  /*Get game state*/
  struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(userdata), "gstate");
  /*Get right tokens, lrnum and tnum*/
  int32_t lrnum = -1;
  int32_t tnum = 0;
    /*Get rigth layer*/
  struct layer* rightlayer = nullptr;
  bool is_new_layer = false;
  for(;;){
    for(int32_t i = 0; i < gstate->m->lrnum; ++i){
      if(gstate->m->l[i].toktype == type && gstate->m->l[i].type == IMG){
        lrnum = i;
        break;
      }
    }
    if(lrnum >= 0){
      break;
    }
    g_print("Layer with type %d not exist\n-Create new layer!\n", type);
    if(!current_scene_add_layer(gstate, IMG, type)){
      g_print("ALLERT! Layer not created!\n");
      return;
    }
    is_new_layer = true;
  }
  rightlayer = &gstate->m->l[lrnum];
    /*Get right tokens*/
  if(is_new_layer){
    tnum = 1;
  }else{
    tnum = rightlayer->rnum + 1;
  }
  struct token* rts = (struct token*)calloc(tnum, sizeof(struct token));
  if(type == TOKEN){
      /*Get right linking tokens*/
    if(!is_new_layer){
      for(int32_t i = 0; i < rightlayer->rnum; ++i){
        for(int32_t j = 0; j < gstate->tnum; ++j){
          if(rightlayer->tokid[i] == gstate->tokens[j].id){
            rts[i] = gstate->tokens[j];
          }
        }
      }
    }
      /*Set not linking token*/
    rts[tnum - 1] = gstate->tokens[id];
  }else if(type == MAP){
      /*Get right register tokens*/
    if(!is_new_layer){
      for(int32_t i = 0; i < rightlayer->rnum; ++i){
        for(int32_t j = 0; j < gstate->mnum; ++j){
          if(rightlayer->tokid[i] == gstate->maps[j].id){
            rts[i] = gstate->maps[j];
          }
        }
      }
    }
    /*Set not linking token*/
    rts[tnum - 1] = gstate->maps[id];
  }else{
    g_print("Invalid tokens type: %d\n", type);
    return;
  }
  /*Set data*/
  data->userdata = userdata;
  data->righttoks = rts;
  data->lrnum = lrnum;
  data->tnum = tnum;
  data->is_new_layer = is_new_layer;
  /*Create new task*/
  GTask* task = g_task_new(nullptr, nullptr, set_layer_texture_atlas, data);
  g_task_set_task_data(task, data, nullptr);
  g_task_run_in_thread(task, work_with_atlas);
  g_object_unref(task);
}

static void work_with_atlas(GTask* task, gpointer source, gpointer taskdata, GCancellable* cancelablle)
{
  foratlas* data = (foratlas*)taskdata;
  /*Get game state*/
  struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(data->userdata), "gstate");
  /*Made atlas*/
  char* atlas_path = nullptr;
    /*Define atlas_dir_path*/
  char* atlas_dir_path = (char*)calloc(512, sizeof(char));
  snprintf(atlas_dir_path, 512, "saves/%s/presets/atlas/%d", gstate->gamename, gstate->current_map_id);
    /*Create atlas dir*/
  makedir(atlas_dir_path);
    /*Get only unique tokens*/
  struct token* uniques = (struct token*)calloc(data->tnum, sizeof(struct token));
  for(int32_t i = 0; i < data->tnum; ++i){
    uniques[i].id = -1;
  }
  int32_t unum = 0;
  for(int32_t i = 0; i < data->tnum; ++i){
    bool exist = false;
    for(int32_t j = 0; j < unum; ++j){
      if(data->righttoks[i].id == uniques[j].id){
        exist = true;
        break;
      }
    }
    if(!exist){
      uniques[unum++] = data->righttoks[i];
    }
  }
    /*Create atlas*/
  float w, h;
  get_path_to_atlas(atlas_dir_path, data->lrnum, uniques, unum, &w, &h, &atlas_path);
  /*Asociate uniques UV with righttoks UV*/
  for(int32_t i = 0; i < data->tnum; ++i){
    for(int32_t j = 0; j < unum; ++j){
      if(uniques[j].id == data->righttoks[i].id){
        set_token_UV_from_token(&data->righttoks[i], &uniques[j]);
        break;
      }
    }
  }
  /*Free*/
  free(uniques);
  free(atlas_dir_path);
  /*Set for layer data*/
  forlayer* fl = (forlayer*)calloc(1, sizeof(forlayer));
  if(!fl){
    g_print("Can not allocate forlayer data\n");
    free(atlas_path);
    return;
  }
  fl->atlas_path = atlas_path;
  fl->not_link_tok = &data->righttoks[data->tnum - 1];
  fl->w = w;
  fl->h = h;
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
    g_warning("Something gone wrong, forlayer structure not recived: %s\n", err->message);
    g_error_free(err);
    g_free(data);
    free(fl);
    return;
  }
  /*Make current gl context*/
  struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(data->userdata), "gstate");
  GtkGLArea* gl_area = GTK_GL_AREA(g_object_get_data(G_OBJECT(data->userdata), "gl_area"));
  gtk_gl_area_make_current(gl_area);
  /*Get current game scene*/
  struct gamescene* current = &gstate->gscns[gstate->current_map_id - 1];
  struct layer* L = &current->l[data->lrnum];
  struct renderer* R = &gstate->m->ren[data->lrnum];
  /*Link not linking token*/
  if(data->righttoks[0].type == TOKEN){
    /*Get token*/
    struct token* t = fl->not_link_tok;
    /*Put token into layer*/
    struct rectangle rect;
    rect_set_XYWH(&rect, gstate->m->cam.cam_x, gstate->m->cam.cam_y,
                  current->l[0].gridstep, current->l[0].gridstep);
    rect_set_RGBA(&rect, 1.0f, 1.0f, 1.0f, 1.0f);
    rect_set_UV(&rect, t->u0, t->v0, t->u1, t->v1);
    layer_add_rect(L, &rect);
    layer_link_last_rect_with_token(L, t);
  }else if(data->righttoks[0].type == MAP){
    /*Get map*/
    struct token* m = fl->not_link_tok;
    /*Put token into layer*/
    struct rectangle rect;
    rect_set_XYWH(&rect, gstate->m->cam.cam_x, gstate->m->cam.cam_y, fl->w, fl->h);
    rect_set_RGBA(&rect, 1.0f, 1.0f, 1.0f, 1.0f);
    rect_set_UV(&rect, m->u0, m->v0, m->u1, m->v1);
    layer_add_rect(L, &rect);
    layer_link_last_rect_with_token(L, m);
  }
  /*Create new renderer*/
  if(data->is_new_layer){
    createrenderer(R, L, IMG);
  }
  /*Layer update texture atlas*/
  layer_update_texture_atlas(&current->l[current->lrnum - 1], fl->atlas_path);
  int32_t n = L->rnum < data->tnum ? L->rnum : data->tnum;
  for(int32_t i = 0; i < n; ++i){
    rect_set_UV(&L->rects[i],
                data->righttoks[i].u0, data->righttoks[i].v0,
                data->righttoks[i].u1, data->righttoks[i].v1);
  }
  /*Free*/
  free(data->righttoks);
  g_free(data);
  free(fl->atlas_path);
  free(fl);
}
