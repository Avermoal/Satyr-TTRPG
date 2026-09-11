#include "Map/camera.h"

#include <stdlib.h>

void zoom_at(struct camera* cam, float screen_x, float screen_y,
             float widget_width, float widget_height, float new_zoom)
{
  if(new_zoom <= 0.0f){
    return;
  }
  /*Get world coords*/
  float world_x = screen_to_world_x(cam->cam_x, screen_x, widget_width, cam->zoom);
  float world_y = screen_to_world_y(cam->cam_y, screen_y, widget_height, cam->zoom);
  /*Set new zoom*/
  cam->zoom = new_zoom;
  /*Get new camera coords*/
  cam->cam_x = world_x - (screen_x - widget_width*0.5f)/cam->zoom;
  cam->cam_y = world_y - (screen_y - widget_height*0.5f)/cam->zoom;
}

void getortho(float ortho*, float l, float r, float b, float t, float n, float f)
{
  memset(ortho, 0, 16*sizeof(float));

  ortho[0] = 2.0f / (r - l);
  ortho[5] = 2.0f / (t - b);
  ortho[10] = -2.0f / (f - n);
  ortho[12] = -(r + l) / (r - l);
  ortho[13] = -(t + b) / (t - b);
  ortho[14] = -(f + n) / (f - n);
  ortho[15] = 1.0f;
}
