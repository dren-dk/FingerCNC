#include <avr/interrupt.h>
#include <string.h>

#include "inputs.h"
#include "events.h"
#include "motor.h"

#include "board.h"

/*
| Function  | Define         | AVR | Arduino | AKA.    |
|-----------|----------------|-----|---------|---------|
| X-Min     | LIMIT_X_MIN    | PE5 | D3      |         |
| X-Max     | LIMIT_X_MAX    | PE4 | D2      |         |
| Y-Min     | LIMIT_Y_MIN    | PJ1 | D14     |         |
| Enc-btn   | ENCODER_BUTTON | PC2 | D35     | EXP1-2  |
| Enc-a     | ENCODER_A      | PC6 | D31     | EXP2-3  |
| Enc-b     | ENCODER_B      | PC4 | D33     | EXP2-5  |
| Stop      | STOP           | PG0 | D41     | EXP2-8  |

Ok, so I wanted to have all the inputs trigger interrupts,
so I could have a nice state machine driven by the inputs
with very little overhead, but the RAMPs folks didn't make
that possible as most of the inputs ended up on non-interrupt
pins.

Oh well, I guess I'll just have to set up a timer interrupt
to do input polling and debouncing.

The main process is quite slow as it's loaded down by having
to update the LCD, so the timer interrupt feeds events to the
UI via a circular buffer, see events.h for how to consume the
events.
*/ 

int16_t encoderPosition = 0;
int8_t encoderDirection = 0;

int16_t getEncoderPosition() {
  return encoderPosition;
}

void setEncoderPosition(uint16_t pos) {
  encoderPosition = pos;
}

#define BOUNCE_LIMIT 3
int8_t bouncy[EVENTS_USED];


void updateDebounceCounters(Event event, uint8_t raw) {
  int8_t old = bouncy[event];
  
  if (raw) {
    if (old <= 0) {
      bouncy[event] = 1;
    } else if (old < BOUNCE_LIMIT) {
      bouncy[event] = old+1;
      if (old == BOUNCE_LIMIT-1) {
	if (event == EVENT_ENC_A) {
	  // Ignore
	} else if (event == EVENT_ENC_B) {
	  encoderDirection = +1;
	} else {
	  addEvent(event);
	  if (event == EVENT_STOP) {
	    enableXMotor(0); 
	  }
	}
      }
    }
  } else {
    if (old >= 0) {
      bouncy[event] = -1;
    } else if (old > -BOUNCE_LIMIT) {
      bouncy[event] = old-1;
      if (old == -(BOUNCE_LIMIT-1)) {
	if (event == EVENT_ENC_A) {
	  encoderPosition += encoderDirection;
	  addEvent(event);
	  
	} else if (event == EVENT_ENC_B) {
	  encoderDirection = -1;
	  
	} else {
	  addEvent(event | EVENT_ACTIVE);	  
	}
      }
    }    
  }
} 

ISR(TIMER0_COMPA_vect) {
  motorPoll();
  updateDebounceCounters(EVENT_ENC_BTN, GPREAD(ENCODER_BUTTON));
  updateDebounceCounters(EVENT_ENC_A,   GPREAD(ENCODER_A));
  updateDebounceCounters(EVENT_ENC_B,   GPREAD(ENCODER_B));
  updateDebounceCounters(EVENT_STOP,    GPREAD(STOP));
  updateDebounceCounters(EVENT_X_MIN,   readXMin());
  updateDebounceCounters(EVENT_X_MAX,   readXMax());
  updateDebounceCounters(EVENT_Y_MIN,   readYMin());  
}

uint8_t readXMin() {
  return GPREAD(LIMIT_X_MIN);
}

uint8_t readXMax() {
  return GPREAD(LIMIT_X_MAX);
}

uint8_t readYMin() {
#ifdef INVERT_Y_MIN
  return !GPREAD(LIMIT_Y_MIN);
#else
  return GPREAD(LIMIT_Y_MIN);
#endif
}

/*
| Function  | Define         | AVR | Arduino | AKA.    |
|-----------|----------------|-----|---------|---------|
| X-Min     | LIMIT_X_MIN    | PE5 | D3      |         |
| X-Max     | LIMIT_X_MAX    | PE4 | D2      |         |
| Y-Min     | LIMIT_Y_MIN    | PJ1 | D14     |         |
| Enc-btn   | ENCODER_BUTTON | PC2 | D35     | EXP1-2  |
| Enc-a     | ENCODER_A      | PC6 | D31     | EXP2-3  |
| Enc-b     | ENCODER_B      | PC4 | D33     | EXP2-5  |
| Stop      | STOP           | PG0 | D41     | EXP2-8  |
  
*/

void inputsInit() {
  // Ensure that all the inputs are inputs.
  GPINPUT(LIMIT_X_MIN);
  GPINPUT(LIMIT_X_MAX);
  GPINPUT(LIMIT_Y_MIN);
  GPINPUT(ENCODER_BUTTON);
  GPINPUT(ENCODER_A);
  GPINPUT(ENCODER_B);
  GPINPUT(STOP);
  

  // And that pullups are enabled
  GPSET(LIMIT_X_MIN);
  GPSET(LIMIT_X_MAX);
  GPSET(ENCODER_BUTTON);
  GPSET(ENCODER_A);
  GPSET(ENCODER_B);
  GPSET(STOP);

#ifdef PULLUP_Y_MIN
  GPSET(LIMIT_Y_MIN);
#endif
  
  // Set up timer 0 to poll inputs
  TCNT0 = 0;
  OCR0A = 15; // 16 -> ~1 ms between interrupts @ 16 MHz
  TCCR0A = _BV(WGM01); // CTC: Count from 0 to OCR0A   
  TCCR0B = _BV(CS00) | _BV(CS02);   // clk/1024 = 15.625 kHz = 64 us/count
  TIMSK0 |= _BV(OCIE0A); // Interrupt on compare A
  sei();
}


