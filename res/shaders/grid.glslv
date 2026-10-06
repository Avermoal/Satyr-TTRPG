#version 320 es

uniform mat4 u_proj;

uniform vec2 u_camera_pos;
uniform float u_grid_step;

uniform float u_visible_half_x;
uniform float u_visible_half_y;

void main(void)
{
  int half_lines_x = int(ceil(u_visible_half_x / u_grid_step)) + 1;
  int half_lines_y = int(ceil(u_visible_half_y / u_grid_step)) + 1;

  int total_lines_x = half_lines_x * 2 + 1;
  int total_lines_y = half_lines_y * 2 + 1;
  int total_vertices = (total_lines_x + total_lines_y) * 2;

  int vid = gl_VertexID;

  bool is_vertical = vid < (total_lines_x * 2);

  vec2 world_pos = vec2(0.0f);

  if(is_vertical){
    int line_index = vid / 2;
    int point_index = vid % 2;

    float offset_x = (float(line_index) - float(half_lines_x)) * u_grid_step;

    float snapped_cam_x = floor(u_camera_pos.x / u_grid_step) * u_grid_step;
    world_pos.x = snapped_cam_x + offset_x;

    float start_y = u_camera_pos.y - float(half_lines_y) * u_grid_step;
    float end_y   = u_camera_pos.y + float(half_lines_y) * u_grid_step;
    world_pos.y = (point_index == 0) ? start_y : end_y;
  }else{
    int h_vid = vid - (total_lines_x * 2);
    int line_index = h_vid / 2;
    int point_index = h_vid % 2;

    float offset_y = (float(line_index) - float(half_lines_y)) * u_grid_step;

    float snapped_cam_y = floor(u_camera_pos.y / u_grid_step) * u_grid_step;
    world_pos.y = snapped_cam_y + offset_y;

    float start_x = u_camera_pos.x - float(half_lines_x) * u_grid_step;
    float end_x   = u_camera_pos.x + float(half_lines_x) * u_grid_step;
    world_pos.x = (point_index == 0) ? start_x : end_x;
  }

  gl_Position = u_proj * vec4(world_pos, 0.0f, 1.0f);
}
