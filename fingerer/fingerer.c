#include <ctype.h>
#include <inttypes.h>

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>

#include <avr/wdt.h> 
#include <avr/interrupt.h>
#include <avr/eeprom.h> 
#include <avr/pgmspace.h>
#include <avr/eeprom.h> 

#include "mstdio.h"

#include "lcd.h"
#include "inputs.h"
#include "events.h"


void led(char on) {
  if (on) {
    PORTB &=~ _BV(PB7);   
  } else {
    PORTB |= _BV(PB7);   
  }
}

void motorInit() {
  DDRD |= _BV(PD7);
  DDRF |= _BV(PF0);
  DDRF |= _BV(PF1);
  DDRF |= _BV(PF2);
  DDRF |= _BV(PF6);
  DDRF |= _BV(PF7);
}

void step(char motor) {
  if (motor) {
    PORTF |= _BV(PF0);
    PORTF &= ~_BV(PF0);    
  } else {
    PORTF |= _BV(PF6);
    PORTF &= ~_BV(PF6);    
  }
}

void initBoard() {
  wdt_enable(WDTO_4S);
  DDRB  |= _BV(PB7);  // LED output
  led(1);

  //  initADC();
  muartInit();
  motorInit();
  lcdInit();
  inputsInit();

  mprintf(PSTR("Power up\n"));
}

#define nop() asm volatile("nop")

void loopSleep() {
  //  _delay_us(350);
  _delay_us(200);
}

int main(void) {
  initBoard();

  uint8_t ledState = 0;
  while (1) {
    wdt_reset();
    
    Event event = takeEvent();
    if (event != EVENT_NONE) {
      mprintf(PSTR("Got event: %d\r\n"), event);
      led(ledState);
      ledState = !ledState;
    }

    lcdHello();
  }
}
