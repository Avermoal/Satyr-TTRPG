#include "EventsSystem/event_system.h"

#include "Map/map.h"
#include "Map/camera.h"

#define MOTION_SPEED 1000.0f

gboolean on_key_pressed(GtkEventControllerKey* controller, guint keyval,
                        guint keycode, GdkModifierType state, gpointer userdata)
{
  /*Get map pointer*/
  struct map** m = g_object_get_data(G_OBJECT(userdata), "map_p");
  if(!m){
    return GDK_EVENT_STOP;
  }
  /*Motion*/
  if(GDK_KEY_W == keyval || GDK_KEY_w == keyval){
    (*m)->evdata.key_w = true;
  }
  if(GDK_KEY_S == keyval || GDK_KEY_s == keyval){
    (*m)->evdata.key_s = true;
  }
  if(GDK_KEY_A == keyval || GDK_KEY_a == keyval){
    (*m)->evdata.key_a = true;
  }
  if(GDK_KEY_D == keyval || GDK_KEY_d == keyval){
    (*m)->evdata.key_d = true;
  }
  return GDK_EVENT_PROPAGATE;
}

gboolean on_key_released(GtkEventControllerKey* controller, guint keyval,
                        guint keycode, GdkModifierType state, gpointer userdata)
{
  /*Get map pointer*/
  struct map** m = g_object_get_data(G_OBJECT(userdata), "map_p");
  if(!m){
    return GDK_EVENT_STOP;
  }
  /*Motion*/
  if(GDK_KEY_W == keyval || GDK_KEY_w == keyval){
    (*m)->evdata.key_w = false;
  }
  if(GDK_KEY_S == keyval || GDK_KEY_s == keyval){
    (*m)->evdata.key_s = false;
  }
  if(GDK_KEY_A == keyval || GDK_KEY_a == keyval){
    (*m)->evdata.key_a = false;
  }
  if(GDK_KEY_D == keyval || GDK_KEY_d == keyval){
    (*m)->evdata.key_d = false;
  }
  return GDK_EVENT_PROPAGATE;
}

void eventpoll(struct map* m)
{
  /*Get camera, event data and delta time*/
  struct camera* cam = &m->cam;
  struct eventdata* evdata = &m->evdata;
  float deltatime = (float)m->evdata.deltatime;
  /*Motion*/
  if(evdata->key_w){
    cam->cam_y -= MOTION_SPEED * deltatime;
  }
  if(evdata->key_s){
    cam->cam_y += MOTION_SPEED * deltatime;
  }
  if(evdata->key_d){
    cam->cam_x += MOTION_SPEED * deltatime;
  }
  if(evdata->key_a){
    cam->cam_x -= MOTION_SPEED * deltatime;
  }
}
