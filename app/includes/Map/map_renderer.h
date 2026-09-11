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
  GLint u_proj_loc;
  GLint u_tex_loc;
};

void createrenderer(struct renderer* ren, struct layer* l, uint32_t REN_TYPE);

void destroyrenderer(struct renderer* ren);

void renderlayer(struct renderer* ren, struct layer* l, float* ortho);

#define/*MAP_MAP_RENDERER_H*/
