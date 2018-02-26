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
#include "fingerjoints.h"

void testFingers() {
  FingerJoints fj;

  initFingerJoints(&fj, getConfigValue(C_FINGER_WIDTH), getConfigValue(C_BOARD),
                     getConfigValue(C_SPACE), getConfigValue(C_HOME_OFFSET),
                     getConfigValue(C_BLADE_WIDTH), getConfigValue(C_STRIDE));

  for (int i=0;i<20;i++) {
    
    P("width:%ld board:%d hp:%ld kerf:%ld stride:%ld cuts:%d advance:%ld ssp:%ld cut:%d pos:%ld\n",
      fj.fingerWidth, fj.board, fj.homePos,
      fj.kerf, fj.stridePct, fj.cutsPerSlot, fj.advancePerCut, fj.spaceStartPos,
      fj.currentCut, fj.currentPos
    );
    
    fingerAvance(&fj);
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
  testFingers();
  
  L("Ready\n");
  
  while (1) {
    uiHandleEvents();
  }
}
