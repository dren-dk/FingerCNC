#include <avr/pgmspace.h>

#include "uart.h"
#include "uiCut.h"

#include "lcd.h"
#include "inputs.h"

#include "ui.h"
#include "uiEdit.h"
#include "uiSetup.h"
#include "motor.h"


/*
Finger size: 12.123 mm
A/B Hole
Status
Config
*/

uint8_t armed = 0;
uint8_t cutOpt = 0;
int32_t pos = 0;

void uiCut(Event event) {
  static uint8_t moveArmed = 0;

  if (event == (EVENT_X_AT_MIN | EVENT_ACTIVE)) {
    moveArmed = 0;
    pos = 0;
  }

  if (moveArmed && yHome && !motorMoving()) {
    pos += 2000;    
    motorMoveTo(pos);
    moveArmed = 0;
  }

  if (!yHome) {
    moveArmed = 1;
  }
    
}

void uiUpdateCut(Event event) {
  if (event == EVENT_NONE) {
    // Init
    armed=0; 
  }

  if (armed) {
    uiCut(event);
  } else {
    // Check if at arm location    
  }
    
  u8g2_ClearBuffer(&u8g2);
  u8g2_SetDrawColor(&u8g2, 1);
  u8g2_SetFont(&u8g2, u8g2_font_6x12_te);  

  uint16_t x = 1+drawTextP(1, 1, 0, PSTR("Cut "));

  char tmp[10];
  ConfigParam* fingerWidth = getConfigParam(C_FINGER_WIDTH);
  cpValueToString(fingerWidth, tmp, 1);
  x += drawText(x, 1, cutOpt==0, tmp);
  
  u8g2_SendBuffer(&u8g2);  
}
