#version 320 es

uniform mat4 u_proj;

uniform vec2 u_camera_pos;
uniform float u_grid_step;

uniform float u_visible_half_x;
uniform float u_visible_half_y;

void main(void)
{
  float min_x = u_camera_pos.x - u_visible_half_x;
  float max_x = u_camera_pos.x + u_visible_half_x;
  float min_y = u_camera_pos.y - u_visible_half_y;
  float max_y = u_camera_pos.y + u_visible_half_y;

  int n_min_x = int(floor(min_x / u_grid_step)) - 1;
  int n_max_x = int(ceil(max_x / u_grid_step)) + 1;
  int n_min_y = int(floor(min_y / u_grid_step)) - 1;
  int n_max_y = int(ceil(max_y / u_grid_step)) + 1;

  int total_lines_x = n_max_x - n_min_x + 1;
  int total_lines_y = n_max_y - n_min_y + 1;

  int total_vertices = (total_lines_x + total_lines_y) * 2;

  int vid = gl_VertexID;

  if (vid >= total_vertices) {
    gl_Position = vec4(0.0, 0.0, -1.0, 1.0);
    return;
  }

  bool is_vertical = vid < (total_lines_x * 2);

  vec2 world_pos = vec2(0.0f);

  if (is_vertical) {
    int line_index = vid / 2;
    int point_index = vid % 2;

    float world_x = float(n_min_x + line_index) * u_grid_step;
    world_pos.x = world_x;

    world_pos.y = (point_index == 0) ? min_y - u_grid_step : max_y + u_grid_step;
  } else {
    int h_vid = vid - (total_lines_x * 2);
    int line_index = h_vid / 2;
    int point_index = h_vid % 2;

    float world_y = float(n_min_y + line_index) * u_grid_step;
    world_pos.y = world_y;

    world_pos.x = (point_index == 0) ? min_x - u_grid_step : max_x + u_grid_step;
  }

  gl_Position = u_proj * vec4(world_pos, 0.0f, 1.0f);
}
