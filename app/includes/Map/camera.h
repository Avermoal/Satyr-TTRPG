#ifndef MAP_CAMERA_H
#define MAP_CAMERA_H

struct camera{
  float cam_x, cam_y;
  float zoom;
  float ortho[16];
};

inline float screen_to_world_x(float screen_x, float widget_width, float cam_x, float zoom)
{
  return cam_x + (screen_x - widget_width*0.5f)/zoom;
}

inline float screen_to_world_y(float screen_y, float widget_height, float cam_y, float zoom)
{
  return cam_y + (screen_y - widget_height*0.5f)/zoom;
}

void zoom_at(struct camera* cam, float screen_x, float screen_y,
             float widget_width, float widget_height, float new_zoom);

void getortho(float ortho*, float l, float r, float b, float t, float n, float f);


#endif/*MAP_CAMERA_H*/
