#include "debug.h"
#include "board.h"

/*
This is just a simple way of twiddling some ports that can be hooked up to
a logic analyzer, for timing stuff.

| Function  | Define         | AVR | Arduino | AKA.    |
|-----------|----------------|-----|---------|---------|
| LED       | LED            | PB7 | D13     |         |
| Debug 0   | DEBUG_0        | PG5 | D4      | Servo 4 |
| Debug 1   | DEBUG_1        | PE3 | D5      | Servo 3 |
| Debug 2   | DEBUG_2        | PH3 | D6      | Servo 2 |
*/


void initDebug() {
  GPOUTPUT(DEBUG_0);
  GPOUTPUT(DEBUG_1);
  GPOUTPUT(DEBUG_2);
  GPOUTPUT(LED);
  setDebug(0);
}

void setDebug0(uint8_t on) {
  GPWRITE(DEBUG_0, on);
}

void setDebug1(uint8_t on) {
  GPWRITE(DEBUG_1, on);
}

void setDebug2(uint8_t on) {
  GPWRITE(DEBUG_2, on);
}

void setDebug(uint8_t debug) {
  setDebug0(debug & 1);
  setDebug1(debug & 2);
  setDebug2(debug & 4);
}

void setLed(uint8_t on) {
  GPWRITE(LED, on);
}

