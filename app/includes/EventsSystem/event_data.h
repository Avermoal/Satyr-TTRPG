#ifndef EVENTSYSTEM_EVENT_DATA_H
#define EVENTSYSTEM_EVENT_DATA_H

#define TO_CURRENT_TIME(frametime) (double) frametime / 1000000.0

struct eventdata{
  double drag_last_dx;
  double drag_last_dy;

  double mousex;
  double mousey;

  double last_frame_time;
  double deltatime;

  bool key_w;
  bool key_s;
  bool key_d;
  bool key_a;
};

#endif/*EVENTSYSTEM_EVENT_DATA_H*/
