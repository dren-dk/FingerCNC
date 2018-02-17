#include <avr/wdt.h> 
#include <stdio.h>

#include "lcd.h"
#include "inputs.h"
#include "events.h"
#include "ui.h"
#include "motor.h"
#include "uart.h"
#include "debug.h"
#include "config.h"

#include "avr8gpio.h"

/*
 | Debug 0   | PG5 | D4      | Servo 4 |         |
 | Debug 1   | PE3 | D5      | Servo 3 |         |

 */
void test() {
    GPWRITE(GPE3, 1);
    //PORTE |= _BV(PE5);
    for (uint8_t i=0;i<5;i++) {
      PORTG |=  _BV(PG5);
      PORTG &=~ _BV(PG5);    
    }
    //PORTE &=~ _BV(PE5);
    GPWRITE(GPE3, 0);
    for (uint8_t i=0;i<10;i++) {
      GPWRITE(GPG5, i);
      GPWRITE(GPG5, !i);
    }
}


int main(void) {
  wdt_enable(WDTO_4S);
  initDebug();
  setLed(1);

  uartInit();
  
  L("Powering up\n");
  initConfig();
  motorInit();
  lcdInit();
  inputsInit();
  uiInit();

  setLed(0);
  L("Ready\n");
  
  while (1) {
    test();
    uiHandleEvents();
  }
}
