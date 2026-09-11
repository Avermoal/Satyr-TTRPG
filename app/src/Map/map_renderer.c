#include <Map/map_renderer.h>

#include "Map/map_data.h"
#include "Map/shader_loader.h"
#include "Map/texture.h"
#include "Map/camera.h"

static void cr_grid(struct renderer* ren, struct layer* l);
static void cr_img(struct renderer* ren, struct layer* l);
static void cr_other(struct renderer* ren, struct layer* l);

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
    /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!MAYBE (WHERE SHOULD CREATE AND DESTROY TEXTURES?)*/
    ren->u_tex_loc = -1;
  }
}

void renderlayer(struct renderer* ren, struct layer* l, float* ortho)
{
  if(!ren || !l){
    ren;
  }
  if(ren->prog == 0 || l->rnum == 0){
    return;
  }
  /*Set usage*/
  glUseProgram(ren->prog);
  glBindVertexArray(ren->vao);
  glBindBuffer(GL_ARRAY_BUFFER, ren->vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(struct vertex)*VERTEX_PER_RECT*l->rnum, l->rects);
  glUniformMatrix4fv(ren->u_proj_loc, 1, GL_FALSE, ortho);
  /*DRAW*/
  glActiveTexture(GL_TEXTURE0);
  for(){
    glBindTexture(GL_TEXTURE_2D, l->tex);
    glUniform1i(ren->u_tex_loc, 0);
    glDrawArrays(GL_TRIANGLES, 0, VERTEX_PER_RECT*l->rnum);
  }
}

static void cr_grid(struct renderer* ren, struct layer* l)
{

}

static void cr_img(struct renderer* ren, struct layer* l)
{
  load_shader_program(&ren->prog);
  ren->u_proj_loc = glGetUniformLocation(ren->prog, "u_proj");
  ren->u_tex_loc = glGetUniformLocation(ren->prog, "u_tex");
  glGenVertexArrays(1, &ren->vao);
  glBindVertexArray(ren->vao);
  glGenBuffers(1, &ren->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, ren->vbo);
  /*Set buffer data*/
  glBufferData(GL_ARRAY_BUFFER, sizeof(struct vertex)*VERTEX_PER_RECT*l->rnum, NULL, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*)offsetof(struct vertex, x));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*)offsetof(struct vertex, u));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*)offsetof(struct vertex, r));
  glEnableVertexAttribArray(2);
  /*Unbind vertex array*/
  glBindVertexArray(0);
}

static void cr_other(struct renderer* ren, struct layer* l)
{

}
