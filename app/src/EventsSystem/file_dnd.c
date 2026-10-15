#include "EventsSystem/file_dnd.h"

#include <stdint.h>
#include <string.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_resize2.h>
#include <stb_image_write.h>

#include "Map/map.h"
#include "Map/map_data.h"

static void linear_resize_img(GFile* img);

gboolean on_files_drop(GtkDropTarget* target, const GValue* value,
                       double x, double y, gpointer userdata)
{
  if(G_VALUE_HOLDS(value, GDK_TYPE_FILE_LIST)){
    GdkFileList* filelist = g_value_get_boxed(value);
    if(filelist != nullptr){
      GSList* files = gdk_file_list_get_files(filelist);
      /*Copy files into dir!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
      const char* dest_dir_path = "saves/game1/presets/textures/game_scene_1/1";
      const char* future_atlas_path = "saves/game1/presets/atlas/game_scene_1";
      GFile* destdir = g_file_new_for_path(dest_dir_path);
      for(GSList* l = files; l != nullptr; l = l->next){
        GFile* sourcefile = G_FILE(l->data);
        char* basename = g_file_get_basename(sourcefile);
        GFile* destfile = g_file_get_child(destdir, basename);
        g_file_copy(sourcefile, destfile, G_FILE_COPY_OVERWRITE, nullptr, nullptr, nullptr, nullptr);
        linear_resize_img(destfile);
        /*Free*/
        g_free(basename);
        g_object_unref(destfile);
      }
      /*Set current gl area*/
      GtkGLArea* area = GTK_GL_AREA(g_object_get_data(G_OBJECT(userdata), "gl_area"));
      gtk_gl_area_make_current(area);
      /*Call layer set texture atlas*/
      struct map** map_p = (struct map**)g_object_get_data(G_OBJECT(userdata), "map_p");
      layer_update_texture_atlas(&(*map_p)->l[IMG], dest_dir_path, future_atlas_path, IMG);
      return TRUE;
    }
  }
  return FALSE;
}

static void linear_resize_img(GFile* img)
{
  const int percentage = 50;
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
