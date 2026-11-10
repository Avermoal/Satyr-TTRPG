#ifndef EVENTSYSTEM_EVENT_DATA_H
#define EVENTSYSTEM_EVENT_DATA_H

#define TO_CURRENT_TIME(frametime) (double) frametime / 1000000.0

struct eventdata{
  float drag_off_x;
  float drag_off_y;

  int32_t draglayer;
  int32_t drag_rid;

  bool dragging;
  bool draggingscene;

  float mousex;
  float mousey;

  double last_frame_time;
  double deltatime;

  bool key_w;
  bool key_s;
  bool key_d;
  bool key_a;
};

static inline void init_event_data(struct eventdata* evdata)
{
  if(!evdata){
    return;
  }
  evdata->drag_off_y = 0.0f;
  evdata->drag_off_y = 0.0f;
  evdata->draglayer = 0;
  evdata->drag_rid = 0;
  evdata->dragging = false;
  evdata->draggingscene = false;
  evdata->mousex = 0.0f;
  evdata->mousey = 0.0f;
  evdata->last_frame_time = 0.0;
  evdata->deltatime = 0.0;
  evdata->key_w = false;
  evdata->key_s = false;
  evdata->key_d = false;
  evdata->key_a = false;
}

#endif/*EVENTSYSTEM_EVENT_DATA_H*/
