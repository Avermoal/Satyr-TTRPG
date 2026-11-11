#include <Map/map_renderer.h>

#include <stdlib.h>
#include <math.h>

#include "Map/map_data.h"
#include "Map/shader_loader.h"
#include "Map/texture.h"
#include "Map/camera.h"

static void cr_grid(struct renderer* ren, struct layer* l);
static void cr_img(struct renderer* ren, struct layer* l);

static void setup_vertex_attribs(void);
static bool renderer_ensure_capacity(struct renderer* ren, int32_t rnum);

bool fill_render_data(struct renderdata* rens, struct layer* ls, int32_t lrnum,
                      uint32_t types[lrnum], const char* const textures_paths[lrnum])
{
  if(!rens || lrnum <= 0 || ls == nullptr){
    return false;
  }
  struct renderer* ren = (struct renderer*)calloc(lrnum, sizeof(struct renderer));
  struct texture* tex = (struct texture*)calloc(lrnum, sizeof(struct texture));
  if(!ren || !tex){
    free(ren);
    free(tex);
    return false;
  }
  for(int32_t i = 0; i < lrnum; ++i){
    createrenderer(&ren[i], &ls[i], types[i]);
    tex[i].id = load_png(textures_paths[i], &tex[i].width, &tex[i].height);
  }
  rens->ren = ren;
  rens->tex = tex;
  return true;
}

void clear_render_data(struct renderdata* rens, int32_t lrnum)
{
  if(!rens || !rens->ren || !rens->tex){
    return;
  }
  for(int32_t i = 0; i < lrnum; ++i){
    destroyrenderer(&rens->ren[i]);
    deletetexture(&rens->tex[i]);
  }
  rens->ren = nullptr;
  rens->tex = nullptr;
}

bool append_render_data(struct renderdata* rens, struct layer* l, int32_t lrnum,
                        uint32_t type, const char* const texture_path)
{
  if(!rens || !l || lrnum < 0){
    return false;
  }
  struct renderer* ren = (struct renderer*)calloc((size_t)lrnum + 1, sizeof(struct renderer));
  struct texture* tex = (struct texture*)calloc((size_t)lrnum + 1, sizeof(struct texture));
  if(!ren || !tex){
    free(ren);
    free(tex);
    return false;
  }
  for(int32_t i = 0; i < lrnum; ++i){
    ren[i] = rens->ren[i];
    tex[i] = rens->tex[i];
  }
  /*Assosiate renderer and layer*/
  createrenderer(&ren[lrnum], l, type);
  tex[lrnum].id = load_png(texture_path, &tex[lrnum].width, &tex[lrnum].height);
  /*Free old*/
  free(rens->ren);
  free(rens->tex);
  /*Sets*/
  rens->ren = ren;
  rens->tex = tex;
  return true;
}

bool update_render_data_texture(struct renderdata* rens, int32_t lrid, const char* texture_path)
{
  if(!rens || !rens->ren || !rens->tex || lrid < 0){
    return false;
  }
  rens->tex[lrid].id = load_png(texture_path, &rens->tex[lrid].width, &rens->tex[lrid].height);
  return true;
}

void createrenderer(struct renderer* ren, struct layer* l, uint32_t REN_TYPE)
{
  switch(REN_TYPE){
    case GRID:
      cr_grid(ren, l);
      return;

    case IMG:
      cr_img(ren, l);
      return;

    default:
      cr_img(ren, l);
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
    ren->u_grid_step_loc = -1;
    ren->u_grid_opacity_loc = -1;
    ren->u_visible_half_x_loc = -1;
    ren->u_visible_half_y_loc = -1;
    ren->u_camera_pos_loc = -1;
    ren->u_lrnum_loc = -1;
  }
}

void renderlayer(struct renderer* ren, struct texture* tex ,struct layer* l, float lrnum, float* ortho,
                 float ww, float wh, float cam_x, float cam_y, float zoom)
{
  if(!ren || !l || !ortho){
    return;
  }

  if(l->type != GRID){
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
    if(l->isdirty){
      glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(sizeof(struct vertex)*VERTEX_PER_RECT*l->rnum), l->rects);
      l->isdirty = false;
    }
    glUniformMatrix4fv(ren->u_proj_loc, 1, GL_FALSE, ortho);
    glUniform1f(ren->u_lrnum_loc, lrnum);
    /*DRAW*/
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glUniform1i(ren->u_tex_loc, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ren->ibo);
    glDrawElements(GL_TRIANGLES, (GLsizei)(INDEX_PER_RECT * l->rnum), GL_UNSIGNED_INT, NULL);
  }else{
    if(ren->vao == 0){
      return;
    }
    if(ren->prog == 0){
      return;
    }
    float vhalf_x = ww / (2.0f * zoom);
    float vhalf_y = wh / (2.0f * zoom);
    int vhalf_lines_x = (int)ceil(vhalf_x / l->gridstep) + 1;
    int vhalf_lines_y = (int)ceil(vhalf_y / l->gridstep) + 1;
    int totalvert = ((vhalf_lines_x * 2 + 1) + (vhalf_lines_y) * 2 + 1) * 2;
    /*Set usage*/
    glUseProgram(ren->prog);
    glBindVertexArray(ren->vao);
    glUniformMatrix4fv(ren->u_proj_loc, 1, GL_FALSE, ortho);
    glUniform1f(ren->u_grid_step_loc, l->gridstep);
    glUniform1f(ren->u_grid_opacity_loc, l->gridopacity);
    glUniform1f(ren->u_visible_half_x_loc, vhalf_x);
    glUniform1f(ren->u_visible_half_y_loc, vhalf_y);
    glUniform2f(ren->u_camera_pos_loc, cam_x, cam_y);
    /*DRAW*/
    glBindVertexArray(ren->vao);
    glDrawArrays(GL_LINES, 0, totalvert);
    glBindVertexArray(0);
  }
}

static void cr_grid(struct renderer* ren, struct layer* l)
{
  load_shader_program(&ren->prog, GRID);
  ren->u_proj_loc = glGetUniformLocation(ren->prog, "u_proj");
  ren->u_grid_step_loc = glGetUniformLocation(ren->prog, "u_grid_step");
  ren->u_grid_opacity_loc = glGetUniformLocation(ren->prog, "u_grid_opacity");
  ren->u_visible_half_x_loc = glGetUniformLocation(ren->prog, "u_visible_half_x");
  ren->u_visible_half_y_loc = glGetUniformLocation(ren->prog, "u_visible_half_y");
  ren->u_camera_pos_loc = glGetUniformLocation(ren->prog, "u_camera_pos");
  glGenVertexArrays(1, &ren->vao);
  glBindVertexArray(ren->vao);
  glGenBuffers(1, &ren->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, ren->vbo);
  float dummydata = 0.0f;
  glBufferData(GL_ARRAY_BUFFER, sizeof(float), &dummydata, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, 0);
  /*Unbind vertex array*/
  glBindVertexArray(0);
}

static void cr_img(struct renderer* ren, struct layer* l)
{
  load_shader_program(&ren->prog, IMG);
  ren->u_proj_loc = glGetUniformLocation(ren->prog, "u_proj");
  ren->u_tex_loc = glGetUniformLocation(ren->prog, "u_tex");
  ren->u_lrnum_loc = glGetUniformLocation(ren->prog, "u_lrnum");
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
