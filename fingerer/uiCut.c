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



int32_t pos = 0;

void uiUpdateCut(Event event) {

  if (event == EVENT_NONE) {
    pos = 0;
  }
  
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
  
  u8g2_ClearBuffer(&u8g2);
  
  u8g2_SetFont(&u8g2, u8g2_font_6x12_te);
  if (yHome) {
    u8g2_DrawStr(&u8g2, 1, 60, "Cut screen Home");
  } else {
    u8g2_DrawStr(&u8g2, 1, 60, "Cut screen");
  }
  
  u8g2_SendBuffer(&u8g2);  
}
