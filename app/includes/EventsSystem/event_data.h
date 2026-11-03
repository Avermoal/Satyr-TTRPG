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

#endif/*EVENTSYSTEM_EVENT_DATA_H*/
