#include "events.h"
#include <util/atomic.h>

#include "eventnames.cpart"

#define CAPACITY 20
Event queue[CAPACITY];
Event previousEvent = EVENT_NONE;

uint8_t head = 0;
uint8_t tail = 0;

uint8_t eventBufferFull() {
  if (tail == 0) {
    return head == CAPACITY-1;
  } else {
    return head == tail-1;
  }
}

uint8_t eventBufferEmpty() {
  return head == tail;
}

void addEvent(Event event) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    if (eventBufferFull()) {
      return; // Discard overflowing events
    }
    if (!eventBufferEmpty() && previousEvent==event) {
      return; // Discard duplicate events that have not been picked up
    }
    previousEvent = event;
    queue[head++] = event;
    if (head >= CAPACITY) {
      head = 0;
    }
  }
}

Event takeEvent() {
  Event result;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    if (eventBufferEmpty()) {
      result = EVENT_NONE;
    } else {
      result = queue[tail++];
      if (tail >= CAPACITY) {
	tail = 0;
      }
    }
  }
  return result;
}

PGM_P getEventName(Event event) {
  if (event < EVENTS_USED) {
    return (PGM_P)pgm_read_word(&(EVENT_NAMES[event]));
  } else {
    return PSTR("Error");
  }
}
