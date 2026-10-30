#include "EventsSystem/event_system.h"

#include "Map/map.h"
#include "Map/camera.h"

#define MOTION_SPEED 1000.0f

gboolean on_key_pressed(GtkEventControllerKey* controller, guint keyval,
                        guint keycode, GdkModifierType state, gpointer userdata)
{
  /*Get map pointer*/
  struct map* m = (struct map*)g_object_get_data(G_OBJECT(userdata), "map");
  if(!m){
    return GDK_EVENT_STOP;
  }
  GdkDisplay* display = gtk_widget_get_display(GTK_WIDGET(userdata));
  GdkKeymapKey* keys = nullptr;
  guint* keyvals = nullptr;
  int nentries = 0;
  guint latin_keyval = 0;
  /*Get final key*/
  if(gdk_display_map_keycode(display, keycode, &keys, &keyvals, &nentries)){
    for(int i = 0; i < nentries; ++i){
      if(keys[i].group == 0 && keys[i].level == 0){
        latin_keyval = keyvals[i];
        break;
      }
    }
  }
  /*Free*/
  g_free(keyvals);
  g_free(keys);
  /*Get final*/
  guint final_keyval = (latin_keyval != 0) ? latin_keyval : keyval;
  /*Motion*/
  if(GDK_KEY_w == final_keyval){
    m->evdata.key_w = true;
  }
  if(GDK_KEY_s == final_keyval){
    m->evdata.key_s = true;
  }
  if(GDK_KEY_a == final_keyval){
    m->evdata.key_a = true;
  }
  if(GDK_KEY_d == final_keyval){
    m->evdata.key_d = true;
  }
  return GDK_EVENT_PROPAGATE;
}

gboolean on_key_released(GtkEventControllerKey* controller, guint keyval,
                        guint keycode, GdkModifierType state, gpointer userdata)
{
  /*Get map pointer*/
  struct map* m = (struct map*)g_object_get_data(G_OBJECT(userdata), "map");
  if(!m){
    return GDK_EVENT_STOP;
  }
  GdkDisplay* display = gtk_widget_get_display(GTK_WIDGET(userdata));
  GdkKeymapKey* keys = nullptr;
  guint* keyvals = nullptr;
  int nentries = 0;
  guint latin_keyval = 0;
  /*Get final key*/
  if(gdk_display_map_keycode(display, keycode, &keys, &keyvals, &nentries)){
    for(int i = 0; i < nentries; ++i){
      if(keys[i].group == 0 && keys[i].level == 0){
        latin_keyval = keyvals[i];
        break;
      }
    }
  }
  /*Free*/
  g_free(keyvals);
  g_free(keys);
  /*Get final*/
  guint final_keyval = (latin_keyval != 0) ? latin_keyval : keyval;
  /*Motion*/
  if(GDK_KEY_w == final_keyval){
    m->evdata.key_w = false;
  }
  if(GDK_KEY_s == final_keyval){
    m->evdata.key_s = false;
  }
  if(GDK_KEY_a == final_keyval){
    m->evdata.key_a = false;
  }
  if(GDK_KEY_d == final_keyval){
    m->evdata.key_d = false;
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
