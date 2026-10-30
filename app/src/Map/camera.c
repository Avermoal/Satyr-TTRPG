#include "Map/camera.h"

#include <string.h>

#define ZOOM_DIVIDER 50.0f

void zoomat(struct camera* cam, float delta)
{
  /*Calc new zoom*/
  float newzoom = cam->zoom + delta / ZOOM_DIVIDER;
  if(newzoom <= 0.1f || newzoom >= 10.0f){
    return;
  }
  cam->zoom = newzoom;
}

void getortho(float* ortho, float l, float r, float b, float t, float n, float f)
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

void getviewport(struct camera* cam, float ww, float wh, float* l, float* r, float* b, float* t)
{
  float half_w = (ww*0.5f)/cam->zoom;
  float half_h = (wh*0.5f)/cam->zoom;

  if(l){
    *l = cam->cam_x - half_w;
  }
  if(r){
    *r = cam->cam_x + half_w;
  }
  if(t){
    *t = cam->cam_y - half_h;
  }
  if(b){
    *b = cam->cam_y + half_h;
  }
}
