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

#include "stdio.h"

#include "lcd.h"
#include "inputs.h"
#include "events.h"
#include "ui.h"
#include "motor.h"
#include "uart.h"

void led(char on) {
  if (on) {
    PORTB &=~ _BV(PB7);   
  } else {
    PORTB |= _BV(PB7);   
  }
}

void initBoard() {
  wdt_enable(WDTO_4S);
  DDRB  |= _BV(PB7);  // LED output
  led(1);

  uartInit();
  printf_P(PSTR("Powering up\n"));
  motorInit();
  lcdInit();
  inputsInit();
  uiInit();

  led(0);
  printf_P(PSTR("Ready\n"));
}

int main(void) {
  initBoard();
  
  while (1) {
    uiHandleEvents();
  }
}
