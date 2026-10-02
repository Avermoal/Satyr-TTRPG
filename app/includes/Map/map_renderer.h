#ifndef MAP_MAP_RENDERER_H
#define MAP_MAP_RENDERER_H

#include <stdint.h>

#include <epoxy/gl.h>

struct layer;
struct camera;

struct renderer{
  GLuint prog;
  GLuint vao;
  GLuint vbo;
  GLuint ibo;
  GLint u_proj_loc;
  GLint u_tex_loc;
  int32_t rect_cap;
};

void createrenderer(struct renderer* ren, struct layer* l, uint32_t REN_TYPE);

void destroyrenderer(struct renderer* ren);

void renderlayer(struct renderer* ren, struct layer* l, float* ortho);

#endif/*MAP_MAP_RENDERER_H*/
