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

  int32_t selected_tid;
  int32_t selected_toktype;
  int32_t selected_rid;
  int32_t selected_lid;

  bool selecttoken;
  bool selectend;

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
  evdata->drag_off_x = 0.0f;
  evdata->drag_off_y = 0.0f;
  evdata->draglayer = 0;
  evdata->drag_rid = 0;
  evdata->dragging = false;
  evdata->draggingscene = false;
  evdata->selected_tid = 0;
  evdata->selected_toktype = 0;
  evdata->selected_rid = 0;
  evdata->selected_lid = 0;
  evdata->selecttoken = false;
  evdata->selectend = false;
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
