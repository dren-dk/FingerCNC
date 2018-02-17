#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

#include "inputs.h"
#include "events.h"
#include "motor.h"

#include "board.h"

/*
| Function  | AVR | Arduino | AKA.    | int     |
|-----------|-----|---------|---------|---------|
| X-Min     | PE5 | D3      |         | INT 5   |
| X-Max     | PE4 | D2      |         | INT 4   |
| Y-Min     | PJ1 | D14     |         | PCI 10  |
| Enc-btn   | PC2 | D35     | EXP1-2  |         |
| Enc-a     | PC6 | D31     | EXP2-3  |         |
| Enc-b     | PC4 | D33     | EXP2-5  |         |
| Stop      | PG0 | D41     | EXP2-8  |         |

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
  updateDebounceCounters(EVENT_ENC_BTN, PINC & _BV(PC2));
  updateDebounceCounters(EVENT_ENC_A,   PINC & _BV(PC6));
  updateDebounceCounters(EVENT_ENC_B,   PINC & _BV(PC4));
  updateDebounceCounters(EVENT_STOP,    PING & _BV(PG0));
  updateDebounceCounters(EVENT_X_MIN,   readXMin());
  updateDebounceCounters(EVENT_X_MAX,   readXMax());
  updateDebounceCounters(EVENT_Y_MIN,   readYMin());  
}

uint8_t readXMin() {
  return PINE & _BV(PE5);
}

uint8_t readXMax() {
  return PINE & _BV(PE4);
}

uint8_t readYMin() {
  return !(PINJ & _BV(PJ1)); // Anders
}

void inputsInit() {
  // Ensure that all the inputs are inputs.
  DDRC &=~ (_BV(PC2)|_BV(PC4)|_BV(PC6));
  DDRE &=~ (_BV(PE5)|_BV(PE4));
  DDRG &=~ _BV(PG0);
  DDRJ &=~ _BV(PJ1);

  // And that pullups are enabled
  PORTC |= _BV(PC2)|_BV(PC4)|_BV(PC6);
  PORTG |= _BV(PG0);
  PORTE |= _BV(PE5) | _BV(PE4);

  PORTJ |= _BV(PJ1); // Pullup for Anders
  
  // Set up timer 0 to poll inputs
  TCNT0 = 0;
  OCR0A = 15; // 16 -> ~1 ms between interrupts @ 16 MHz
  TCCR0A = _BV(WGM01); // CTC: Count from 0 to OCR0A   
  TCCR0B = _BV(CS00) | _BV(CS02);   // clk/1024 = 15.625 kHz = 64 us/count
  TIMSK0 |= _BV(OCIE0A); // Interrupt on compare A
  sei();
}


