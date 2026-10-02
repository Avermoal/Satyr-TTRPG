#include <Map/map_renderer.h>

#include "Map/map_data.h"
#include "Map/shader_loader.h"
#include "Map/texture.h"
#include "Map/camera.h"

static void cr_grid(struct renderer* ren, struct layer* l);
static void cr_img(struct renderer* ren, struct layer* l);
static void cr_other(struct renderer* ren, struct layer* l);

static void setup_vertex_attribs(void);
static bool renderer_ensure_capacity(struct renderer* ren, int32_t rnum);

void createrenderer(struct renderer* ren, struct layer* l, uint32_t REN_TYPE)
{
  switch(REN_TYPE){
    case GRID:
      cr_grid(ren, l);
      return;

    case IMG:
      cr_img(ren, l);
      return;

    case OTHER:
      cr_other(ren, l);
      return;

    default:
      return;
  }
}

void destroyrenderer(struct renderer* ren)
{
  if(ren){
    if(ren->ibo){
      glDeleteBuffers(1, &ren->ibo);
    }
    if(ren->vbo){
      glDeleteBuffers(1, &ren->vbo);
    }
    if(ren->vao){
      glDeleteVertexArrays(1, &ren->vao);
    }
    if(ren->prog){
      glDeleteProgram(ren->prog);
    }
    ren->prog = 0;
    ren->u_proj_loc = -1;
    ren->u_tex_loc = -1;
  }
}

void renderlayer(struct renderer* ren, struct layer* l, float* ortho)
{
  if(!ren || !l || !ortho){
    return;
  }
  if(ren->vao == 0 || ren->vbo == 0 || ren->ibo == 0){
    return;
  }
  if(ren->prog == 0 || l->rnum == 0 || !l->rects){
    return;
  }
  if(!renderer_ensure_capacity(ren, l->rnum)){
    return;
  }
  /*Set usage*/
  glUseProgram(ren->prog);
  glBindVertexArray(ren->vao);
  glBindBuffer(GL_ARRAY_BUFFER, ren->vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(sizeof(struct vertex)*VERTEX_PER_RECT*l->rnum), l->rects);
  glUniformMatrix4fv(ren->u_proj_loc, 1, GL_FALSE, ortho);
  /*DRAW*/
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, l->tex.id);
  glUniform1i(ren->u_tex_loc, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ren->ibo);
  glDrawElements(GL_TRIANGLES, (GLsizei)(INDEX_PER_RECT * l->rnum), GL_UNSIGNED_INT, NULL);
}

static void cr_grid(struct renderer* ren, struct layer* l)
{
  load_shader_program(&ren->prog, IMG);/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
  ren->u_proj_loc = glGetUniformLocation(ren->prog, "u_proj");
  ren->u_tex_loc = glGetUniformLocation(ren->prog, "u_tex");
  glGenVertexArrays(1, &ren->vao);
  glBindVertexArray(ren->vao);
  glGenBuffers(1, &ren->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, ren->vbo);
  /*Set buffer data*/
  setup_vertex_attribs();
  /*Gen indices buffer*/
  glGenBuffers(1, &ren->ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ren->ibo);
  renderer_ensure_capacity(ren, l->rnum);
  /*Unbind vertex array*/
  glBindVertexArray(0);
}

static void cr_img(struct renderer* ren, struct layer* l)
{
  load_shader_program(&ren->prog, IMG);
  ren->u_proj_loc = glGetUniformLocation(ren->prog, "u_proj");
  ren->u_tex_loc = glGetUniformLocation(ren->prog, "u_tex");
  glGenVertexArrays(1, &ren->vao);
  glBindVertexArray(ren->vao);
  glGenBuffers(1, &ren->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, ren->vbo);
  /*Set buffer data*/
  setup_vertex_attribs();
  /*Gen indices buffer*/
  glGenBuffers(1, &ren->ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ren->ibo);
  renderer_ensure_capacity(ren, l->rnum);
  /*Unbind vertex array*/
  glBindVertexArray(0);
}

static void cr_other(struct renderer* ren, struct layer* l)
{

}

static void setup_vertex_attribs(void)
{
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*)offsetof(struct vertex, x));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*)offsetof(struct vertex, u));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*)offsetof(struct vertex, r));
  glEnableVertexAttribArray(2);
}

static bool renderer_ensure_capacity(struct renderer* ren, int32_t rnum)
{
  if(!ren || rnum < 0){
    return false;
  }
  if(rnum == 0){
    return true;
  }
  if(ren->vbo == 0 || ren->ibo == 0){
    return false;
  }
  if(ren->rect_cap >= rnum){
    return true;
  }
  int32_t newcap = ren->rect_cap;
  if(newcap == 0){
    newcap = 64;
  }
  while(newcap < rnum){
    if(newcap > INT32_MAX / 2){
      newcap = rnum;
      break;
    }
    newcap *= 2;
  }
  if(newcap < rnum){
    newcap = rnum;
  }
  uint32_t* indices = (uint32_t *)malloc((size_t)newcap * INDEX_PER_RECT * sizeof(uint32_t));
  if(!indices){
    return false;
  }
  for(int32_t i = 0; i < newcap; ++i){
    uint32_t base = (uint32_t)(i * VERTEX_PER_RECT);
    size_t off = (size_t)i * INDEX_PER_RECT;
    indices[off + 0] = base + 0;
    indices[off + 1] = base + 1;
    indices[off + 2] = base + 2;
    indices[off + 3] = base + 2;
    indices[off + 4] = base + 3;
    indices[off + 5] = base + 0;
  }
  GLsizeiptr vsize = (GLsizeiptr)(sizeof(struct vertex) * VERTEX_PER_RECT * newcap);
  GLsizeiptr isize = (GLsizeiptr)((size_t)newcap * INDEX_PER_RECT * sizeof(uint32_t));
  /*Set buffers*/
  glBindBuffer(GL_ARRAY_BUFFER, ren->vbo);
  glBufferData(GL_ARRAY_BUFFER, vsize, NULL, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ren->ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, isize, indices, GL_DYNAMIC_DRAW);
  /*Free*/
  free(indices);
  /*Set*/
  ren->rect_cap = newcap;
  return true;
}
