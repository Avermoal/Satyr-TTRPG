#include "EventsSystem/event_system.h"

#include "UI/UI.h"
#include "UI/DMs_UI.h"
#include "GameState/game_state.h"
#include "Map/map.h"
#include "Map/map_data.h"
#include "Map/camera.h"

#define MOTION_SPEED 1000.0f

static float mousex(struct camera* cam, float x, float ww);
static float mousey(struct camera* cam, float y, float wh);

static bool mouse_on_token(struct layer* l, float x, float y, int32_t* tokid, int32_t* rid);
static bool mouse_on_map(struct layer* l, float x, float y, int32_t* tokid, int32_t* rid);

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

gboolean onscroll(GtkEventControllerScroll* scroll, double dx, double dy, gpointer userdata)
{
  /*Get gstate*/
  struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(userdata), "gstate");
  if(!gstate || !gstate->m){
    return FALSE;
  }
  if(!gstate->m->evdata.draggingscene){
    zoomat(&gstate->m->cam, (float)dy);
  }
  return TRUE;
}

void onmotion(GtkEventControllerMotion* motion, double x, double y, gpointer userdata)
{
  struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(userdata), "gstate");
  if(!gstate){
    return;
  }
  /*Gets*/
  GtkWidget* widget  = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(motion));
  /*Get widget width and height*/
  float ww = (float)gtk_widget_get_width(widget);
  float wh = (float)gtk_widget_get_height(widget);
  struct camera* cam = &gstate->m->cam;
  /*Calc mouse pos in world coord*/
  gstate->m->evdata.mousex = mousex(cam, (float)x, ww);
  gstate->m->evdata.mousey = mousey(cam, (float)y, wh);
  /*Smooth motion*/
  struct eventdata* evdata = &gstate->m->evdata;
  if(evdata->dragging && !evdata->draggingscene){
    layer_move_rect(&gstate->m->l[evdata->draglayer], evdata->drag_rid,
                    evdata->mousex + evdata->drag_off_x,
                    evdata->mousey + evdata->drag_off_y);
  }
  if(evdata->draggingscene && !evdata->dragging){
    movecamera(cam, cam->cam_x -evdata->mousex + evdata->drag_off_x, cam->cam_y -evdata->mousey + evdata->drag_off_y);
  }
}

void on_drag_begin(GtkGestureDrag* gesture, double x, double y, gpointer userdata)
{
  GtkWidget* overlay = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));
  /*1 - LMB, 3 - RMB*/
  guint button = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(gesture));
  GtkWidget* drag_btn = GTK_WIDGET(g_object_get_data(G_OBJECT(overlay), "drag_btn"));
  /*Get game state*/
  struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(userdata), "gstate");
  if(!gstate || !gstate->m){
    return;
  }
  struct eventdata* evdata = &gstate->m->evdata;
  if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(drag_btn))){
    /*Drag map*/
    int32_t tokid = -1;
    int32_t rid = -1;
    for(int32_t i = gstate->m->lrnum - 1; i >= 0; --i){
      if(gstate->m->l[i].toktype != MAP){
        continue;
      }
      /*Grab one token*/
      if(mouse_on_map(&gstate->m->l[i], evdata->mousex, evdata->mousey,
                        &tokid, &rid) && tokid != -1 && rid != -1){
        struct rectangle* r = &gstate->m->l[i].rects[rid];
        float cx = r->vert[0].x + 0.5f * rect_get_w(r);
        float cy = r->vert[0].y + 0.5f * rect_get_h(r);
        evdata->drag_off_x = cx - evdata->mousex;
        evdata->drag_off_y = cy - evdata->mousey;
        evdata->draglayer = i;
        evdata->drag_rid = rid;
        evdata->dragging = true;
        evdata->draggingscene = false;
        change_widget_cursor(overlay, "grabbed");
        g_print("GRAB token id=%d layer=%d rid=%d\n", tokid, i, rid);
        break;
      }
    }
    return;
  }
  /*Drag scene*/
  if(button == 3){
    evdata->draggingscene = true;
    evdata->dragging = false;
    struct camera* cam = &gstate->m->cam;
    evdata->drag_off_x = evdata->mousex;
    evdata->drag_off_y = evdata->mousey;
    return;
  }
  if(button != 1){
    return;
  }
  /*Drag token*/
  int32_t tokid = -1;
  int32_t rid = -1;
  for(int32_t i = gstate->m->lrnum - 1; i >= 0; --i){
    if(gstate->m->l[i].toktype != TOKEN){
      continue;
    }
    /*Grab one token*/
    if(mouse_on_token(&gstate->m->l[i], evdata->mousex, evdata->mousey,
                      &tokid, &rid) && tokid != -1 && rid != -1){
      struct rectangle* r = &gstate->m->l[i].rects[rid];
      float cx = r->vert[0].x + 0.5f * rect_get_w(r);
      float cy = r->vert[0].y + 0.5f * rect_get_h(r);
      evdata->drag_off_x = cx - evdata->mousex;
      evdata->drag_off_y = cy - evdata->mousey;
      evdata->draglayer = i;
      evdata->drag_rid = rid;
      evdata->dragging = true;
      evdata->draggingscene = false;
      change_widget_cursor(overlay, "grabbed");
      g_print("GRAB token id=%d layer=%d rid=%d\n", tokid, i, rid);
      break;
    }
  }
}

void on_drag_end(GtkGestureDrag* gesture, double x, double y, gpointer userdata)
{
  GtkWidget* overlay = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));
  change_widget_cursor(overlay, "default");
  /*Set evdata to default*/
  struct gamestate* gstate = (struct gamestate*)g_object_get_data(G_OBJECT(userdata), "gstate");
  if(gstate && gstate->m){
    gstate->m->evdata.dragging = false;
    gstate->m->evdata.draggingscene = false;
  }
}

void eventpoll(struct map* m)
{
  if(!m){
    return;
  }
  /*Get camera and delta time*/
  struct camera* cam = &m->cam;
  struct eventdata* evdata = &m->evdata;
  float deltatime = (float)m->evdata.deltatime;
  if(deltatime >= 1000.0f){
    return;
  }
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

static inline float mousex(struct camera* cam, float x, float ww)
{
  return cam->cam_x + (x - ww * 0.5f) / cam->zoom;
}

static inline float mousey(struct camera* cam, float y, float wh)
{
  return cam->cam_y + (y - wh * 0.5f) / cam->zoom;
}

static bool mouse_on_token(struct layer* l, float x, float y, int32_t* tokid, int32_t* rid)
{
  /*ONLY IF MAPS LAYERS AND CURRENT GSCN LAYERS IS SAME*/
  if(l->toktype == TOKEN){
    return mouse_XY_in_layer_token(x, y, l, tokid, rid);
  }
  return false;
}

static bool mouse_on_map(struct layer* l, float x, float y, int32_t* tokid, int32_t* rid)
{
  if(l->toktype == MAP){
    return mouse_XY_in_layer_rects(x, y, l, tokid, rid);
  }
  return false;
}
