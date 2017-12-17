#pragma once

typedef enum {
  EVENT_ENC_BTN = 0,
  EVENT_ENC_A = 1,
  EVENT_ENC_B = 2,
  EVENT_STOP = 3,
  
  EVENT_Y_MIN = 4,  
  
  EVENT_NOT = 128,
  EVENT_NONE = 255
} Event;

void addEvent(Event event);
Event takeEvent();
