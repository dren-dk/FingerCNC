#include <stdio.h>

#include "uart.h"

#include "uiHome.h"
#include "lcd.h"
#include "inputs.h"

#include "ui.h"
#include "uiEdit.h"
#include "uiSetup.h"
#include "motor.h"


void uiUpdateHome(Event event) {
  if (event == (EVENT_ENC_BTN|EVENT_ACTIVE)) {
    if (yHome) {
      uiSetScreen(HOMING_SCREEN);
      return;
    }
  }

  u8g2_ClearBuffer(&u8g2);
  u8g2_DrawXBMP(&u8g2, 0, 0, LOGO_WIDTH, LOGO_HEIGHT, LOGO);

  u8g2_SetFont(&u8g2, u8g2_font_6x12_te);

  if (stop) {
    u8g2_DrawStr(&u8g2, 1, 60, "Release STOP");
  } else if (yHome) {
    u8g2_DrawStr(&u8g2, 1, 60, "Press button to home");
  } else {
    u8g2_DrawStr(&u8g2, 1, 60, "Move sled home");
  }
    
  u8g2_SendBuffer(&u8g2);
}

void uiUpdateHoming(Event event) {

  if (event == EVENT_NONE) {
    motorHome(); // Start homing routine
    
  } else if (event == (EVENT_X_AT_MIN | EVENT_ACTIVE)) {
    uiSetScreen(CUT_SCREEN);
    return;
  }

  u8g2_ClearBuffer(&u8g2);
  u8g2_DrawXBMP(&u8g2, 0, 0, LOGO_WIDTH, LOGO_HEIGHT, LOGO);

  u8g2_SetFont(&u8g2, u8g2_font_6x12_te);
  
  u8g2_DrawStr(&u8g2, 1, 60, "Hands-off, homing X");
    
  u8g2_SendBuffer(&u8g2);
}
