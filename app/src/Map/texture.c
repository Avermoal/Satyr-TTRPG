#include "Map/texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <gtk/gtk.h>
#include <epoxy/gl.h>

uint32_t load_png(const char* path, int32_t* width, int32_t* height)
{
  *width = 0;
  *height = 0;
  int32_t channels = 0;
  uint32_t id = 0;
  /*Flip stbi loading image configure*/
  stbi_set_flip_vertically_on_load(true);
  /*Load image*/
  uint8_t* img = stbi_load(path, width, height, &channels, 4);
  if(!img){
    g_printerr("LOAD_TEXTURE: Texture by path: %s, can't loading", path);
    return id;
  }
  /*Texture in GL context*/
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
  /*Free section*/
  stbi_image_free(img);
  return id;
}

void bindtexture(uint32_t id)
{
  glBindTexture(GL_TEXTURE_2D, id);
}

void unbindtexture(void)
{
  glBindTexture(GL_TEXTURE_2D, 0);
}

void deletetexture(struct texture* tex)
{
  if(tex){
    glDeleteTextures(1, &tex->id);
    tex->width = 0;
    tex->height = 0;
  }
}


