#include "EventsSystem/file_dnd.h"

#include <stdint.h>
#include <string.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_resize2.h>
#include <stb_image_write.h>

#include "GameState/game_state.h"
#include "Tokens/tokens.h"

typedef struct{
  GdkFileList* filelist;
  gpointer userdata;
  int32_t toktype;
}drop_task_data;

static void remove_drag_overlay(GtkDropTargetAsync* target);
static void work_with_files(GTask* task, gpointer source, gpointer taskdata, GCancellable* cancelablle);
static void on_value_read_ready_toks(GObject* source, GAsyncResult* res, gpointer userdata);
static void on_value_read_ready_maps(GObject* source, GAsyncResult* res, gpointer userdata);
static void free_drop_task_data(gpointer data);
static void linear_resize_img(GFile* img, int32_t toktype);

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
  if(formats && gdk_content_formats_contain_mime_type(formats, "text/plain")){
    /*НЕЗАБЫТЬ что при перетаскивании токена надо кидать на слой токенов а карты на слой карты*/

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
  /*Output size*/
  int outw = (w * percentage) / 100;
  int outh = (h * percentage) / 100;
  /*Output image*/
  uint8_t* outi = calloc(1, (size_t)outw * outh * ch * sizeof(uint8_t));
  /*Output stride*/
  int inputstride = w * ch;
  int outputstride = outw * ch;

  if(filename[file_name_length - 3] == 'p'){
    stbir_resize_uint8_linear(i, w, h, inputstride, outi, outw, outh, outputstride, STBIR_RGBA);
    stbi_write_png(file, outw, outh, ch, outi, outputstride);
  }else{
    stbir_resize_uint8_linear(i, w, h, inputstride, outi, outw, outh, outputstride, STBIR_RGB);
    stbi_write_jpg(file, outw, outh, ch, outi, 100);
  }
  /*Free*/
  stbi_image_free(outi);
  stbi_image_free(i);
  g_free(filename);
  g_free(file);
}
