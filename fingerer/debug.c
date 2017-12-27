#include <avr/io.h>

#include "debug.h"
#include "gpio.h"

/*
This is just a simple way of twiddling some ports that can be hooked up to
a logic analyzer, for timing stuff.

| Debug 0   | PG5 | D4      | Servo 4 |         |
| Debug 1   | PE3 | D5      | Servo 3 |         |
| Debug 2   | PH3 | D6      | Servo 2 |         |
| LED       | PB7 | D13     | LED     |         |
*/

void initDebug() {
  DDRG |= _BV(PG5);
  DDRE |= _BV(PE3);
  DDRH |= _BV(PH3);
  DDRB |= _BV(PB7);
  setDebug(0);
}

void setDebug0(uint8_t on) {
  GPIO(G, 5, on);
}

void setDebug1(uint8_t on) {
  GPIO(E, 3, on);
}

void setDebug2(uint8_t on) {
  GPIO(H, 3, on);
}

void setDebug(uint8_t debug) {
  setDebug0(debug & 1);
  setDebug1(debug & 2);
  setDebug2(debug & 4);
}

void setLed(uint8_t on) {
  GPIO(B, 7, on);
}

