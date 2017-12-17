#include "events.h"
#include <util/atomic.h>

#define CAPACITY 20
Event queue[CAPACITY];

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
      return;
    }
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
