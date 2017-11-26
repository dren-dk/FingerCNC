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

  mprintf(PSTR("Power up\n"));
}

#define nop() asm volatile("nop")

void loopSleep() {
  //  _delay_us(350);
  _delay_us(200);
}

/*
  Symbol -> Modulation -> Periods  -> Steps
 */

const char* DATA = "Test";
const int BIT_PERIOD = 2*200*16;
const char MARK = 0;
const char SPACE = 1;

int main(void) {
  initBoard();

  int bit = 100;
  int ch = 100;

  // int frame = 0;
  char motor = 0;
  int len = 0;
  
  while (1) {
    mputs("Hello\r\n");
      
    //led(frame++ & 128);
    loopSleep();
    wdt_reset();

    if (--len < 0) {
        lcdHello();
      if (++bit > 10) {
	if (++ch > sizeof(DATA)) {
	  ch  = 0;
	}
	bit = 0;      
      }
      
      if (bit == 0) {         // Start bit
	len = BIT_PERIOD;
	motor = MARK; 
      } else if (bit == 9) {  // Stop bits
	len = BIT_PERIOD*2;
	motor = SPACE;
      } else if (bit == 10) { // Spacing
	len = BIT_PERIOD*5;
	motor = SPACE;
      } else {
	len = BIT_PERIOD;
	motor = (_BV(bit-1) & DATA[ch]) ? 1:0;
      }
    }

    led(motor);
    
    //    step(motor);
  }
}
