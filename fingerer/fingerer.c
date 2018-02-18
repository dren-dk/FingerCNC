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
    uiHandleEvents();
  }
}
