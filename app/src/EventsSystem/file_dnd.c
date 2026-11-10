#include "EventsSystem/file_dnd.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_resize2.h>
#include <stb_image_write.h>

#include "UI/DMs_UI.h"
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
  int32_t max_atlas_size;
  /*SNAPSHOT*/
  char* gamename;
  int32_t tnum;
  struct token* tokens;
  int32_t mnum;
  struct token* maps;
}drop_task_data;

typedef struct{
  gpointer userdata;
  GdkDrop* drop;
  char buffer[64];
}mime_read_ctx;

typedef struct{
  gpointer userdata;
  char** path;
  char** cache;
  char* atlas_path;                 /*Atlas path not need to be free*/
  struct token* ts;                 /*Fictive token*/
  int32_t tnum;
  int32_t pnum;
  int32_t toktype;
}drop_task_data_final;

typedef struct{
  gpointer userdata;
  struct token* righttoks;
  int32_t lrnum;
  int32_t tnum;
  bool is_new_layer;
  /*SNAPSHOT*/
  char* gamename;
  int32_t current_map_id;
  int32_t max_atlas_size;
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
static void linear_resize_img(GFile* img, int iscache);

static void on_mime_type_read_toks_done(GObject* source, GAsyncResult* res, gpointer userdata);
static void on_stream_read_done(GObject* sourcestream, GAsyncResult* res, gpointer userdata);
static void mime_read_ctx_free(mime_read_ctx* ctx);

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
    /*Update mouse pos while dnd*/
    struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(userdata), "gstate");
    if(gstate){
      GtkWidget* gl_area = GTK_WIDGET(g_object_get_data(G_OBJECT(userdata), "gl_area"));
      GtkWidget* widget  = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(target));

      double lx = x, ly = y;
      if(gl_area && widget && widget != gl_area){
        graphene_point_t in = GRAPHENE_POINT_INIT((float)x, (float)y), out;
        if(gtk_widget_compute_point(widget, gl_area, &in, &out)){ lx = out.x; ly = out.y; }
      }
      float ww = (float)gtk_widget_get_width (gl_area ? gl_area : widget);
      float wh = (float)gtk_widget_get_height(gl_area ? gl_area : widget);
      struct camera* cam = &gstate->m->cam;
      gstate->m->evdata.mousex = cam->cam_x + ((float)lx - ww * 0.5f) / cam->zoom;
      gstate->m->evdata.mousey = cam->cam_y + ((float)ly - wh * 0.5f) / cam->zoom;
    }
    /*Async*/
    mime_read_ctx* ctx = g_new0(mime_read_ctx, 1);
    ctx->drop = g_object_ref(drop);
    ctx->userdata = userdata;
    const char* mimes[] = {"text/plain", nullptr};
    gdk_drop_read_async(drop, mimes,
                        G_PRIORITY_DEFAULT, nullptr,
                        on_mime_type_read_toks_done, ctx);
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
  const char* saves = "saves";
  const char* dest = "presets/textures";
  const char* cachedest = "presets/textures/cache";
  char dest_dir_path[256];
  char dest_dir_cache[256];
  if(data->toktype == TOKEN){
    snprintf(dest_dir_path, sizeof(dest_dir_path), "%s/%s/%s/tokens", saves, data->gamename, dest);
  }else if(data->toktype == MAP){
    snprintf(dest_dir_path, sizeof(dest_dir_path), "%s/%s/%s/maps", saves, data->gamename, dest);
    snprintf(dest_dir_cache, sizeof(dest_dir_cache), "%s/%s/%s", saves, data->gamename, cachedest);
  }
    /*Copy*/
  GFile* destdir = g_file_new_for_path(dest_dir_path);
  if(!destdir){
    return;
  }
  GFile* cachedir = nullptr;
  if(data->toktype == MAP){
    cachedir = g_file_new_for_path(dest_dir_cache);
    if(!cachedir){
      g_object_unref(destdir);
      return;
    }
  }
  drop_task_data_final* datafinal = (drop_task_data_final*)malloc(sizeof(drop_task_data_final));
  if(!datafinal){
    g_slist_free(files);
    g_object_unref(cachedir);
    g_object_unref(destdir);
    return;
  }
  datafinal->userdata = g_object_ref(data->userdata);
  datafinal->path = (char**)calloc(g_slist_length(files), sizeof(char*));
  if(!datafinal->path){
    g_object_unref(datafinal->userdata);
    free(datafinal);
    datafinal = nullptr;
    g_slist_free(files);
    g_object_unref(cachedir);
    g_object_unref(destdir);
    return;
  }
  datafinal->cache = (char**)calloc(g_slist_length(files), sizeof(char*));
  if(!datafinal->cache){
    g_slist_free(files);
    free(datafinal->path);
    datafinal->path = nullptr;
    g_object_unref(datafinal->userdata);
    free(datafinal);
    g_object_unref(cachedir);
    g_object_unref(destdir);
    return;
  }
  datafinal->ts = nullptr;
  datafinal->tnum = data->tnum;
  datafinal->toktype = data->toktype;
  datafinal->pnum = 0;
  GError* file_copy_err = nullptr;
  for(GSList* l = files; l != nullptr; l = l->next){
    GFile* sourcefile = G_FILE(l->data);
    char* basename = g_file_get_basename(sourcefile);
    bool is_registered_token = false;
    if(data->toktype == TOKEN){
      for(int32_t i = 0; i < data->tnum; ++i){
        if(data->tokens[i].path && strstr(data->tokens[i].path, basename) != nullptr){
          is_registered_token = true;
        }
      }
    }else{
      for(int32_t i = 0; i < data->mnum; ++i){
        if(data->maps[i].path && strstr(data->maps[i].path, basename) != nullptr){
          is_registered_token = true;
        }
      }
    }
    if(!is_registered_token){
      size_t len = strlen(basename);
      if(len < 4 || basename[len - 4] != '.' || !(basename[len - 3] == 'j' || basename[len - 3] == 'p')){
        g_free(basename);
        continue;
      }
      GFile* destfile = g_file_get_child(destdir, basename);
      g_file_copy(sourcefile, destfile, G_FILE_COPY_OVERWRITE, nullptr, nullptr, nullptr, &file_copy_err);
      if(file_copy_err){
        g_print("Some issues with file %s copy: %s\n", basename, file_copy_err->message);
        g_free(basename);
        g_error_free(file_copy_err);
        file_copy_err = nullptr;
        continue;
      }
      GFile* destcache = nullptr;
      if(data->toktype == MAP){
        destcache = g_file_get_child(cachedir, basename);
        g_file_copy(sourcefile, destcache, G_FILE_COPY_OVERWRITE, nullptr, nullptr, nullptr, &file_copy_err);
        if(file_copy_err){
          g_print("Some issues with file %s copy: %s\n", basename, file_copy_err->message);
          g_object_unref(destfile);
          g_free(basename);
          g_error_free(file_copy_err);
          file_copy_err = nullptr;
          continue;
        }
        linear_resize_img(destfile, 0);
        linear_resize_img(destcache, 1);
      }
      /*Set return pointer*/
      datafinal->path[datafinal->pnum] = g_file_get_path(destfile);
      if(data->toktype == MAP){
        datafinal->cache[datafinal->pnum++] = g_file_get_path(destcache);
      }else{
        datafinal->cache[datafinal->pnum++] = nullptr;
      }
      /*Free*/
      if(destcache){
        g_object_unref(destcache);
      }
      g_object_unref(destfile);
    }
    file_copy_err = nullptr;
    /*Free*/
    g_free(basename);
  }
  /*Create new texture atlas for tokens*/
  if(data->toktype == TOKEN){
    /*Get copy of tokens*/
    datafinal->ts = (struct token*)calloc(datafinal->tnum + datafinal->pnum, sizeof(struct token));
    if(!datafinal->ts){
      goto wwf_free;
    }
    for(int32_t i = 0; i < datafinal->tnum; ++i){
      copytoken(&datafinal->ts[i], &data->tokens[i]);
    }
    /*Not registered tokens*/
    for(int32_t i = 0; i < datafinal->pnum; ++i){
      settoken(&datafinal->ts[i + datafinal->tnum], datafinal->toktype, -1, datafinal->path[i], nullptr);
    }
    char* atlas_dir_path = (char*)calloc(512, sizeof(char));
    if(!atlas_dir_path){
      goto wwf_free;
    }
    snprintf(atlas_dir_path, 512, "saves/%s/presets/atlas", data->gamename);
    get_path_to_atlas(data->max_atlas_size, atlas_dir_path, -1, datafinal->ts,
                      datafinal->tnum + datafinal->pnum, nullptr, nullptr, &datafinal->atlas_path);
    if(!datafinal->atlas_path){
      free(atlas_dir_path);
      goto wwf_free;
    }
    free(atlas_dir_path);
  }
  /*Return pointer*/
  g_task_return_pointer(task, datafinal, drop_final_data_free);
  /*Free*/
wwf_free:
  g_slist_free(files);
  if(cachedir){
    g_object_unref(cachedir);
  }
  g_object_unref(destdir);
}

static void on_value_read_ready_toks(GObject* source, GAsyncResult* res, gpointer userdata)
{
  GdkDrop* drop = GDK_DROP(source);
  const GValue* value = gdk_drop_read_value_finish(drop, res, nullptr);
  /*To drop_task_data*/
  struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(userdata), "gstate");
  drop_task_data* data = g_new0(drop_task_data, 1);
  data->tnum = gstate->tnum;
  data->tokens = (struct token*)calloc(data->tnum, sizeof(struct token));
  if(!data->tokens){
    return;
  }
  data->filelist = g_value_dup_boxed(value);
  data->userdata = userdata;
  data->toktype = TOKEN;
    /*Deep copy tokens*/
  for(int32_t i = 0; i < data->tnum; ++i){
    copytoken(&data->tokens[i], &gstate->tokens[i]);
  }
  data->mnum = 0;
  data->maps = nullptr;
  data->max_atlas_size = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(userdata), "max_atlas_size"));
  data->gamename = strdup(gstate->gamename);
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
  struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(userdata), "gstate");
  drop_task_data* data = g_new0(drop_task_data, 1);
  data->mnum = gstate->mnum;
  data->maps = (struct token*)calloc(data->mnum, sizeof(struct token));
  if(!data->maps){
    return;
  }
  data->filelist = g_value_dup_boxed(value);
  data->userdata = userdata;
  data->toktype = MAP;
    /*Deep copy maps*/
  for(int32_t i = 0; i < data->mnum; ++i){
    copytoken(&data->maps[i], &gstate->maps[i]);
  }
  data->tnum = 0;
  data->tokens = nullptr;
  data->max_atlas_size = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(userdata), "max_atlas_size"));
  data->gamename = strdup(gstate->gamename);
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
  GdkDrop* drop = GDK_DROP(source);
  gdk_drop_finish(drop, GDK_ACTION_COPY);
  GError* err = nullptr;
  /*Read pointer*/
  drop_task_data_final* data = (drop_task_data_final*)g_task_propagate_pointer(task, &err);
  if(err){
    g_warning("Final files drop data not readed: %s\n", err->message);
    g_error_free(err);
    if(data){
      drop_final_data_free(data);
    }
    return;
  }
  if(data){
    /*Get game state and dms ui data*/
    struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(data->userdata), "gstate");
    struct DMs_UI* dms_ui = (struct DMs_UI*)g_object_get_data(G_OBJECT(data->userdata), "dms_ui_data");
    /*Register tokens in main thread*/
    for(int32_t i = 0; i < data->pnum; ++i){
      if(data->path[i]){
        if(data->cache[i] != nullptr){
          game_state_register_token(gstate, data->toktype, data->path[i], data->cache[i]);
        }else{
          game_state_register_token(gstate, data->toktype, data->path[i], nullptr);
        }
      }
    }
    /*Update tokens and maps on tokens_box*/
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dms_ui->tokens_btn))){
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dms_ui->tokens_btn), FALSE);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dms_ui->tokens_btn), TRUE);
    }
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dms_ui->maps_btn))){
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dms_ui->maps_btn), FALSE);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dms_ui->maps_btn), TRUE);
    }
    /*Update layers tokens texture atlas in current scene*/
    if(data->toktype == TOKEN){
      GtkGLArea* gl_area = GTK_GL_AREA(g_object_get_data(G_OBJECT(data->userdata), "gl_area"));
      gtk_gl_area_make_current(gl_area);
      if(gtk_gl_area_get_error(gl_area) != nullptr){
        drop_final_data_free(data);
        return;
      }
      /*Update registered tokens UV (old + new)*/
      for(int32_t i = 0; i < data->tnum + data->pnum && i < gstate->tnum; ++i){
        set_token_UV_from_token(&gstate->tokens[i], &data->ts[i]);
      }
      /*Update current map layers texture atlas for tokens*/
      struct layer* L = nullptr;
      for(int32_t i = 0; i < gstate->gscns[gstate->current_map_id - 1].lrnum; ++i){
        L = &gstate->gscns[gstate->current_map_id - 1].l[i];
        if(L->toktype != TOKEN){
          continue;
        }
        /*Reset exists rects UV*/
        for(int32_t j = 0; j < L->rnum; ++j){
          int32_t tid = L->tokid[j];
          if(tid < 0 || tid >= gstate->tnum){
            continue;
          }
          struct token* T = &gstate->tokens[tid];
          rect_set_UV(&L->rects[j], T->u0, T->v0, T->u1, T->v1);
        }
        layer_update_texture_atlas(L, data->atlas_path);
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
    g_free(datafinal->path[i]);
    datafinal->path[i] = nullptr;
    if(datafinal->toktype == MAP){
      g_free(datafinal->cache[i]);
    }
  }
  if(datafinal->toktype == TOKEN){
    for(int32_t i = 0; i < datafinal->tnum + datafinal->pnum; ++i){
      g_free(datafinal->ts[i].path);
      datafinal->ts[i].path = nullptr;
      g_free(datafinal->ts[i].cache);
      datafinal->ts[i].cache = nullptr;
    }
    free(datafinal->atlas_path);
    datafinal->atlas_path = nullptr;
  }
  free(datafinal->ts);
  datafinal->ts = nullptr;
  g_free(datafinal->path);
  datafinal->path = nullptr;
  g_free(datafinal->cache);
  datafinal->cache = nullptr;
  g_object_unref(datafinal->userdata);
  free(datafinal);
  datafinal = nullptr;
}

static void free_drop_task_data(gpointer data)
{
  drop_task_data* subdata = (drop_task_data*)data;
  g_boxed_free(GDK_TYPE_FILE_LIST, subdata->filelist);
  for(int32_t i = 0; i < subdata->tnum; ++i){
    free(subdata->tokens[i].path);
  }
  for(int32_t i = 0; i < subdata->mnum; ++i){
    free(subdata->maps[i].path);
    free(subdata->maps[i].cache);
  }
  free(subdata->tokens);
  free(subdata->maps);
  free(subdata->gamename);
  g_free(subdata);
}

static void linear_resize_img(GFile* img, int iscache)
{
  int percentage = 45 - 40 * iscache;
  /*Get file path*/
  char* file = g_file_get_path(img);
  char* filename = g_file_get_basename(img);
  int file_name_length = strlen(filename);
  /*Get file*/
  int w, h, ch;
  uint8_t* i = stbi_load(file, &w, &h, &ch, 0);
  if(strlen(filename) < 4 || !i){
    goto lri_half_free;
  }
  /*Output size*/
  int outw = (w * percentage) / 100;
  int outh = (h * percentage) / 100;
  /*Output image*/
  uint8_t* outi = calloc(1, (size_t)outw * outh * ch * sizeof(uint8_t));
  if(!outi){
    stbi_image_free(i);
    goto lri_half_free;
  }
  /*Output stride*/
  int inputstride = w * ch;
  int outputstride = outw * ch;
  stbir_pixel_layout layout = (ch == 4) ? STBIR_RGBA : STBIR_RGB;
  if(filename[file_name_length - 3] == 'p'){
    stbir_resize_uint8_srgb(i, w, h, inputstride, outi, outw, outh, outputstride, layout);
    stbi_write_png(file, outw, outh, ch, outi, outputstride);
  }else{
    stbir_resize_uint8_srgb(i, w, h, inputstride, outi, outw, outh, outputstride, layout);
    stbi_write_jpg(file, outw, outh, ch, outi, 100);
  }
  /*Free*/
  free(outi);
  stbi_image_free(i);
lri_half_free:
  g_free(filename);
  g_free(file);
}

/*-------------------TOKENS (MAP) WORK ----------------------*/

static void on_mime_type_read_toks_done(GObject* source, GAsyncResult* res, gpointer userdata)
{
  mime_read_ctx* ctx = (mime_read_ctx*)userdata;
  GdkDrop* drop = ctx->drop;
  GError* err = nullptr;
  const char* out_mime = nullptr;
  GInputStream* stream = gdk_drop_read_finish(drop, res, &out_mime, &err);
  if(!stream){
    g_warning("Read error: %s\n", err ? err->message : "unknown");
    g_clear_error(&err);
    gdk_drop_finish(drop, 0);
    mime_read_ctx_free(ctx);
    return;
  }
  g_input_stream_read_all_async(stream, ctx->buffer, sizeof(ctx->buffer) - 1, G_PRIORITY_DEFAULT,
                                nullptr, on_stream_read_done, userdata);
  g_object_unref(stream);
}

static void on_stream_read_done(GObject* sourcestream, GAsyncResult* res, gpointer userdata)
{
  mime_read_ctx* ctx = (mime_read_ctx*)userdata;
  GdkDrop* drop = ctx->drop;
  GInputStream* stream = G_INPUT_STREAM(sourcestream);
  gsize bytesread = 0;
  GError* err = nullptr;
  /*Read*/
  if(!g_input_stream_read_all_finish(stream, res, &bytesread, &err)){
    g_warning("Read error: %s\n", err ? err->message : "unknown");
    g_clear_error(&err);
    gdk_drop_finish(drop, 0);
    mime_read_ctx_free(ctx);
    return;
  }
  ctx->buffer[bytesread] = '\0';
  char* type_id = ctx->buffer;
  /*Drop finish*/
  gdk_drop_finish(drop, GDK_ACTION_COPY);
  /*Parsing "<type>-<id>" or <sid>*/
  if(strcmp(type_id, "-1") == 0){
    /*To new scene*/
    struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(ctx->userdata), "gstate");
    add_clear_game_scene(gstate);
    mime_read_ctx_free(ctx);
    return;
  }else if(strchr(type_id, '-') == nullptr){
    /*Switch scene*/
    struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(ctx->userdata), "gstate");
    int32_t sid = 0;
    sscanf(type_id, "%d", &sid);
    make_current(gstate, sid);
    mime_read_ctx_free(ctx);
    return;
  }
  char* _id = nullptr;
  int32_t type = strtol(type_id, &_id, 10);
  int32_t id = 0;
  if(_id && *_id){
    sscanf(&_id[1], "%d", &id);
  }
  if(type != MAP && type != TOKEN){
    g_print("Invalid token type: %d\n", type);
    mime_read_ctx_free(ctx);
    return;
  }
  if(id < 0){
    g_print("Invalid token id: %d\n", id);
    mime_read_ctx_free(ctx);
    return;
  }
  /*Get userdata*/
  gpointer userdata_ = ctx->userdata;
  mime_read_ctx_free(ctx);
  /*Get game state*/
  struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(userdata_), "gstate");
  if(type == TOKEN){
    if(id >= gstate->tnum){
      g_print("Token id is out of range: %d\n", id);
      return;
    }
  }else if(type == MAP){
    if(id >= gstate->mnum){
      g_print("Map id is out of range: %d\n", id);
      return;
    }
  }
  /*Allocate memory for foratlas data*/
  foratlas* data = g_new0(foratlas, 1);
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
      g_free(data);
      return;
    }
    is_new_layer = true;
  }
  rightlayer = &gstate->m->l[lrnum];
    /*Get right tokens*/
  if(is_new_layer){
    tnum = 1;
  }else{
    if(type == MAP){
      tnum = rightlayer->rnum + 1;
    }else if(type == TOKEN){
      tnum = 1;
    }
  }
  struct token* rts = (struct token*)calloc(tnum, sizeof(struct token));
  if(!rts){
    g_free(data);
    return;
  }
  if(type == TOKEN){
      /*Set not linking token*/
    copytoken(&rts[tnum - 1], &gstate->tokens[id]);
  }else if(type == MAP){
      /*Get right register tokens*/
    if(!is_new_layer){
      for(int32_t i = 0; i < rightlayer->rnum; ++i){
        for(int32_t j = 0; j < gstate->mnum; ++j){
          if(rightlayer->tokid[i] == gstate->maps[j].id){
            copytoken(&rts[i], &gstate->maps[j]);
          }
        }
      }
    }
    /*Set not linking token*/
    copytoken(&rts[tnum - 1], &gstate->maps[id]);
  }
  /*Set data*/
  data->userdata = g_object_ref(userdata_);
  data->righttoks = (struct token*)calloc(tnum, sizeof(struct token));
  if(!data->righttoks){
    g_print("Error of memory allocation for righttoks\n");
    for(int32_t i = 0; i < tnum; ++i){
      free(rts[i].path);
      free(rts[i].cache);
    }
    free(rts);
    g_object_unref(data->userdata);
    g_free(data);
    return;
  }
  for(int32_t i = 0; i < tnum; ++i){
    copytoken(&data->righttoks[i], &rts[i]);
  }
  data->lrnum = lrnum;
  data->tnum = tnum;
  data->is_new_layer = is_new_layer;
  data->current_map_id = gstate->current_map_id;
  data->max_atlas_size = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(userdata_), "max_atlas_size"));
  data->gamename = strdup(gstate->gamename);
  /*Create new task*/
  GTask* task = g_task_new(nullptr, nullptr, set_layer_texture_atlas, data);
  g_task_set_task_data(task, data, nullptr);
  g_task_run_in_thread(task, work_with_atlas);
  /*Free*/
  for(int32_t i = 0; i < tnum; ++i){
    free(rts[i].path);
    rts[i].path = nullptr;
    free(rts[i].cache);
    rts[i].cache = nullptr;
  }
  free(rts);
  g_object_unref(task);
}

static void mime_read_ctx_free(mime_read_ctx* ctx)
{
  if(!ctx){
    return;
  }
  g_clear_object(&ctx->drop);
  g_free(ctx);
}

static void work_with_atlas(GTask* task, gpointer source, gpointer taskdata, GCancellable* cancelablle)
{
  foratlas* data = (foratlas*)taskdata; /*Read only in new thread*/
  /*Made atlas*/
  char* atlas_path = nullptr;
    /*Define atlas_dir_path*/
  char* atlas_dir_path = (char*)calloc(512, sizeof(char));
  if(data->righttoks[0].type == MAP){
    snprintf(atlas_dir_path, 512, "saves/%s/presets/atlas/%d", data->gamename, data->current_map_id);
  }else if(data->righttoks[0].type == TOKEN){
    atlas_path = (char*)calloc(512, sizeof(char));
    snprintf(atlas_path, 512, "saves/%s/presets/atlas/tokens_atlas.png", data->gamename);
    snprintf(atlas_dir_path, 512, "saves/%s/presets/atlas", data->gamename);
    makedir(atlas_dir_path);
    /*Set for layer data*/
    forlayer* fl = (forlayer*)calloc(1, sizeof(forlayer));
    fl->atlas_path = atlas_path;
    fl->w = 0;
    fl->h = 0;
    fl->not_link_tok = &data->righttoks[data->tnum - 1];
    g_task_return_pointer(task, fl, free);
    /*Free*/
    free(atlas_dir_path);
    return;
  }
    /*Create atlas dir*/
  makedir(atlas_dir_path);
    /*Get only unique tokens (MAP)*/
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
  float* w = (float*)calloc(unum, sizeof(float));
  if(!w){
    free(uniques);
    free(atlas_dir_path);
    g_task_return_pointer(task, nullptr, nullptr);
    return;
  }
  float* h = (float*)calloc(unum, sizeof(float));
  if(!h){
    free(w);
    free(uniques);
    free(atlas_dir_path);
    g_task_return_pointer(task, nullptr, nullptr);
    return;
  }
  get_path_to_atlas(data->max_atlas_size, atlas_dir_path, data->lrnum, uniques, unum, w, h, &atlas_path);
  if(!atlas_path){
    free(w);
    free(h);
    free(uniques);
    free(atlas_dir_path);
    g_task_return_pointer(task, nullptr, nullptr);
    return;
  }
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
    free(w);
    free(h);
    g_task_return_pointer(task, nullptr, nullptr);
    return;
  }
  fl->atlas_path = atlas_path;
  fl->not_link_tok = &data->righttoks[data->tnum - 1];
  fl->w = w[unum - 1];
  fl->h = h[unum - 1];
  g_task_return_pointer(task, fl, free);
  /*Free*/
  free(w);
  free(h);
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
    for(int32_t i = 0; i < data->tnum; ++i){
      free(data->righttoks[i].path);
      free(data->righttoks[i].cache);
    }
    free(data->righttoks);
    free(data->gamename);
    g_object_unref(data->userdata);
    g_free(data);
    free(fl);
    return;
  }
  if(!fl){
    g_warning("Recived nullptr forlayer pointer\n");
    for(int32_t i = 0; i < data->tnum; ++i){
      free(data->righttoks[i].path);
      free(data->righttoks[i].cache);
    }
    free(data->righttoks);
    free(data->gamename);
    g_object_unref(data->userdata);
    g_free(data);
    return;
  }
  /*Make current gl context*/
  struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(data->userdata), "gstate");
  if(!gstate){
    g_print("Scene %d is not current", data->current_map_id);
    for(int32_t i = 0; i < data->tnum; ++i){
      free(data->righttoks[i].path);
      free(data->righttoks[i].cache);
    }
    free(data->righttoks);
    free(data->gamename);
    g_object_unref(data->userdata);
    g_free(data);
    free(fl);
    return;
  }
  GtkGLArea* gl_area = GTK_GL_AREA(g_object_get_data(G_OBJECT(data->userdata), "gl_area"));
  gtk_gl_area_make_current(gl_area);
  if(data->current_map_id != gstate->current_map_id){
    g_print("Scene %d is not current", data->current_map_id);
    for(int32_t i = 0; i < data->tnum; ++i){
      free(data->righttoks[i].path);
      free(data->righttoks[i].cache);
    }
    free(data->righttoks);
    free(data->gamename);
    g_object_unref(data->userdata);
    g_free(data);
    free(fl);
    return;
  }
  /*Get current game scene*/
  struct gamescene* current = &gstate->gscns[data->current_map_id - 1];
  struct layer* L = &current->l[data->lrnum];
  struct renderer* R = &gstate->m->ren[data->lrnum];
  /*Link not linking token*/
  if(data->righttoks[0].type == TOKEN){
    /*Get token*/
    struct token* t = fl->not_link_tok;
    /*Put token into layer*/
    struct rectangle rect;
    rect_set_XYWH(&rect, gstate->m->evdata.mousex - current->l[0].gridstep / 2.0f,
                  gstate->m->evdata.mousey - current->l[0].gridstep / 2.0f,
                  current->l[0].gridstep, current->l[0].gridstep);
    rect_set_RGBA(&rect, 1.0f, 1.0f, 1.0f, 1.0f);
    rect_set_UV(&rect, t->u0, t->v0, t->u1, t->v1);
    if(layer_add_rect(L, &rect)){
      layer_link_last_rect_with_token(L, t);
    }
  }else if(data->righttoks[0].type == MAP){
    /*Get map*/
    struct token* m = fl->not_link_tok;
    /*Put token into layer*/
    struct rectangle rect;
    rect_set_XYWH(&rect, gstate->m->cam.cam_x - fl->w * 0.5f,
                  gstate->m->cam.cam_y - fl->h * 0.5, fl->w, fl->h);
    rect_set_RGBA(&rect, 1.0f, 1.0f, 1.0f, 1.0f);
    rect_set_UV(&rect, m->u0, m->v0, m->u1, m->v1);
    if(layer_add_rect(L, &rect)){
      layer_link_last_rect_with_token(L, m);
    }
  }
  /*Create new renderer*/
  if(data->is_new_layer){
    createrenderer(R, L, IMG);
    layer_update_texture_atlas(L, fl->atlas_path);
  }
  if(data->righttoks[0].type == MAP){
    /*Layer update texture atlas for MAP and rects set UV*/
    int32_t n = L->rnum < data->tnum ? L->rnum : data->tnum;
    layer_update_texture_atlas(L, fl->atlas_path);
    /*Rect set UV*/
    for(int32_t i = 0; i < n; ++i){
      rect_set_UV(&L->rects[i],
                  data->righttoks[i].u0, data->righttoks[i].v0,
                  data->righttoks[i].u1, data->righttoks[i].v1);
    }
  }
  /*Update layers_box*/
  struct DMs_UI* dms_ui = (struct DMs_UI*)g_object_get_data(G_OBJECT(data->userdata), "dms_ui_data");
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dms_ui->settings_btn))){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dms_ui->settings_btn), FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dms_ui->settings_btn), TRUE);
  }
  /*Free*/
  for(int32_t i = 0; i < data->tnum; ++i){
    free(data->righttoks[i].path);
    free(data->righttoks[i].cache);
  }
  free(data->righttoks);
  free(data->gamename);
  g_object_unref(data->userdata);
  g_free(data);
  free(fl->atlas_path);
  free(fl);
}
