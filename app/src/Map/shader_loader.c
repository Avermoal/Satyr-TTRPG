#include "Map/shader_loader.h"

#include <stdlib.h>
#include <stdio.h>

#include <gtk/gtk.h>
#include <epoxy/gl.h>

static int64_t shaderlength(const char* path);
static bool readshader(char* sh_txt, const char* path, const int64_t length);

int load_shader_program(sh_p* id)
{
  *id = 0;
  /*Create shader program*/
  const char* vsh_path = "res/shaders/main.glslv";
  const char* fsh_path = "res/shaders/main.glslf";
  int64_t vsh_length = shaderlength(vsh_path);
  int64_t fsh_length = shaderlength(fsh_path);
    /*Read shaders text*/
  char* vsh_txt = (char*)calloc(vsh_length + 1, sizeof(char));
  char* fsh_txt = (char*)calloc(fsh_length + 1, sizeof(char));
  if(readshader(vsh_txt, vsh_path, vsh_length) && readshader(fsh_txt, fsh_path, fsh_length)){
    *id = create_shader_program(vsh_txt, fsh_txt);
  }
  free(vsh_txt);
  free(fsh_txt);
  return 1;
}

sh_p create_shader_program(const char* vsh_txt, const char* fsh_txt)
{
  sh_p id = 0;
  /*Create shaders id*/
  GLuint vsh_id = glCreateShader(GL_VERTEX_SHADER);
  GLuint fsh_id = glCreateShader(GL_FRAGMENT_SHADER);
  /*Compile shaders*/
  glShaderSource(vsh_id, 1, (const GLchar* const*)&vsh_txt, NULL);
  glCompileShader(vsh_id);
  glShaderSource(fsh_id, 1, (const GLchar* const*)&fsh_txt, NULL);
  glCompileShader(fsh_id);
  /*Shader compile check*/
  GLint success;
  GLchar log[512];
  glGetShaderiv(vsh_id, GL_COMPILE_STATUS, &success);
  if(!success){
    glGetShaderInfoLog(vsh_id, 512, NULL, log);
    g_printerr("SHADER_CREATION: Vertex shader not created:\n");
    g_printerr(log);
    return 0;
  }
  glGetShaderiv(fsh_id, GL_COMPILE_STATUS, &success);
  if(!success){
    glGetShaderInfoLog(fsh_id, 512, NULL, log);
    g_printerr("SHADER_CREATION: Fragment shader not created:\n");
    g_printerr(log);
    return 0;
  }
  /*Create shader program*/
  id = glCreateProgram();
  /*Attaching shaders*/
  glAttachShader(id, vsh_id);
  glAttachShader(id, fsh_id);
  /*Link program*/
  glLinkProgram(id);
  /*Delete shaders*/
  glDetachShader(id, vsh_id);
  glDetachShader(id, fsh_id);
  glDeleteShader(vsh_id);
  glDeleteShader(fsh_id);
  /*Linking check*/
  glGetProgramiv(id, GL_LINK_STATUS, &success);
  if(!success){
    glGetProgramInfoLog(id, 512, NULL, log);
    g_printerr("SHADER_PROGRAM_LINK: Shader program not linking:\n");
    g_printerr(log);
    return 0;
  }
  return id;
}

void destroy_shader_program(const sh_p id)
{
  glDeleteProgram(id);
}

void bind_shader_program(const sh_p id)
{
  glUseProgram(id);
}

void unbind_shader_program(void)
{
  glUseProgram(0);
}

static int64_t shaderlength(const char* path)
{
  FILE* f = fopen(path, "rb");
  if(!f){
    g_printerr("SHADER: can't find shader by path: %s\n", path);
    return 0;
  }
  fseek(f, 0L, SEEK_END);
  int64_t length = ftell(f);
  rewind(f);
  fclose(f);
  return length;
}

static bool readshader(char* sh_txt, const char* path, const int64_t length)
{
  FILE* f = fopen(path, "rb");
  if(!f){
    g_printerr("SHADER: can't read shader by path: %s\n", path);
    return false;
  }
  /*Read shader text*/
  fread(sh_txt, sizeof(char), length, f);
  fclose(f);
  /*Set null-terminator*/
  sh_txt[length] = '\0';
  return true;
}
