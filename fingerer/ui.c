#include <avr/pgmspace.h>
#include <avr/wdt.h> 

#include <stdio.h>

#include "ui.h"
#include "uiEdit.h"
#include "uiSetup.h"

#include "events.h"
#include "lcd.h"
#include "inputs.h"
#include "motor.h"
#include "debug.h"

uint8_t yHome;
uint8_t stop;

UIScreen currentScreen;
void uiUpdate(Event event);

void uiSetScreen(UIScreen screen) {
  currentScreen = screen;
  uiUpdate(EVENT_NONE);
}

void uiUpdateHome(Event event) {
  if (event == (EVENT_ENC_BTN|EVENT_ACTIVE)) {
    if (yHome) {
      currentScreen = HOMING_SCREEN;
      uiUpdate(EVENT_NONE);
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
    currentScreen = CUT_SCREEN;
    uiUpdate(event);
    return;
  }

  u8g2_ClearBuffer(&u8g2);
  u8g2_DrawXBMP(&u8g2, 0, 0, LOGO_WIDTH, LOGO_HEIGHT, LOGO);

  u8g2_SetFont(&u8g2, u8g2_font_6x12_te);
  
  u8g2_DrawStr(&u8g2, 1, 60, "Hands-off, homing X");
    
  u8g2_SendBuffer(&u8g2);
}

void uiUpdateCut(Event event) {

  static uint8_t moveArmed = 0;
  static int32_t pos = 0;

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

void uiUpdate(Event event) {

  if (event == EVENT_Y_MIN || event == (EVENT_Y_MIN | EVENT_ACTIVE)) {
    yHome = !(event & EVENT_ACTIVE);
  }
  
  if (event == EVENT_STOP || event == (EVENT_STOP | EVENT_ACTIVE)) {
    stop = event & EVENT_ACTIVE;
    
    if (stop) {
      printf_P(PSTR("Stop activated\n"));
      currentScreen = HOME_SCREEN;
    }
  }

  if (currentScreen == HOME_SCREEN) {
    uiUpdateHome(event);

  } else if (currentScreen == HOMING_SCREEN) {
    uiUpdateHoming(event);
    
  } else if (currentScreen == CUT_SCREEN) {
    uiUpdateCut(event);
    
  } else if (currentScreen == SETUP_SCREEN) {
    uiUpdateSetup(event);

  } else if (currentScreen == EDIT_SCREEN) {
    uiUpdateEdit(event);
  }
}

void uiInit() {
  currentScreen = SETUP_SCREEN;
  //  currentScreen = HOME_SCREEN;
  uiUpdate(EVENT_NONE);
}

void uiHandleEvents() {
  while (1) {
    wdt_reset();
    
    Event event = takeEvent();
    if (event == EVENT_NONE) {
      return; // No event ready, so we'll just return and let someone else have a go
    }
    
    if (event == EVENT_ENC_A) {
      printf_P(PSTR("Encoder: %d\n"), getEncoderPosition());
    } else {
      printf_P(PSTR("Got event: "));
      if (event & EVENT_ACTIVE) {
	printf_P(PSTR("+"));
      } else {
	printf_P(PSTR("-"));
      }
      
      printf_P(getEventName(event & ~EVENT_ACTIVE));
      printf_P(PSTR("\n"));
    }

    uiUpdate(event);    
  }
  
}




