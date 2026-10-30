#ifndef MAP_CAMERA_H
#define MAP_CAMERA_H

struct camera{
  float cam_x, cam_y;
  float zoom;
  float ortho[16];
};

void zoomat(struct camera* cam, float delta);

void getortho(float* ortho, float l, float r, float b, float t, float n, float f);

void getviewport(struct camera* cam, float ww, float wh, float* l, float* r, float* b, float* t);

#endif/*MAP_CAMERA_H*/
