#ifndef MAP_MAP_RENDERER_H
#define MAP_MAP_RENDERER_H

#include <stdint.h>

#include <epoxy/gl.h>

struct layer;
struct camera;
struct texture;

struct renderer{
  GLuint prog;
  GLuint vao;
  GLuint vbo;
  GLuint ibo;
  GLint u_proj_loc;
  GLint u_tex_loc;
  GLint u_grid_step_loc;
  GLint u_grid_opacity_loc;
  GLint u_visible_half_x_loc;
  GLint u_visible_half_y_loc;
  GLint u_camera_pos_loc;
  GLint u_lrnum_loc;
  int32_t rect_cap;
};

struct renderdata{
  struct renderer* ren;
  struct texture* tex;
};

bool fill_render_data(struct renderdata* rens, struct layer* ls, int32_t lrnum,
                      uint32_t types[lrnum], const char* const textures_paths[lrnum]);

void clear_render_data(struct renderdata* rens, int32_t lrnum);

bool append_render_data(struct renderdata* rens, struct layer* l, int32_t lrnum,
                        uint32_t type, const char* const texture_path);

bool update_render_data_texture(struct renderdata* rens, int32_t lrid, const char* texture_path);

void createrenderer(struct renderer* ren, struct layer* l, uint32_t REN_TYPE);

void destroyrenderer(struct renderer* ren);

void renderlayer(struct renderer* ren, struct texture* tex, struct layer* l, float lrnum, float* ortho,
                 float ww, float wh, float cam_x, float cam_y, float zoom);

#endif/*MAP_MAP_RENDERER_H*/
