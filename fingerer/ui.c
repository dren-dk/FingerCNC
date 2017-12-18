#include <avr/wdt.h> 

#include "mstdio.h"

#include "ui.h"
#include "events.h"
#include "lcd.h"
#include "inputs.h"

typedef enum {
  HOME_SCREEN,
  CUT_SCREEN,
  SETUP_SCREEN
} UIScreen;

UIScreen currentScreen;
void uiUpdate(Event event);


void uiUpdateHome(Event event) {
  static uint8_t yHome;
  if (event & EVENT_Y_MIN) {
    yHome = event & EVENT_ACTIVE;
  }

  if (event == (EVENT_ENC_BTN|EVENT_ACTIVE)) {
    if (yHome) {
      currentScreen = CUT_SCREEN;
      uiUpdate(EVENT_NONE);
      return;
    }
  }

  u8g2_ClearBuffer(&u8g2);
  u8g2_DrawXBMP(&u8g2, 0, 0, LOGO_WIDTH, LOGO_HEIGHT, LOGO);

  u8g2_SetFont(&u8g2, u8g2_font_6x12_te);

  if (yHome) {
    u8g2_DrawStr(&u8g2, 1, 63, "Move sled home");
  } else {
    u8g2_DrawStr(&u8g2, 1, 63, "Press button");
  }
    
  u8g2_SendBuffer(&u8g2);
}

void uiUpdateCut(Event event) {
  u8g2_ClearBuffer(&u8g2);
  
  u8g2_SetFont(&u8g2, u8g2_font_6x12_te);
  u8g2_DrawStr(&u8g2, 1, 63, "Cut screen");

  u8g2_SendBuffer(&u8g2);  
}

void uiUpdateSetup(Event event) {
  u8g2_ClearBuffer(&u8g2);
  
  u8g2_SetFont(&u8g2, u8g2_font_6x12_te);
  u8g2_DrawStr(&u8g2, 1, 63, "Setup screen");

  u8g2_SendBuffer(&u8g2);  
}

void uiUpdate(Event event) {
  if (currentScreen == HOME_SCREEN) {
    uiUpdateHome(event);

  } else if (currentScreen == CUT_SCREEN) {
    uiUpdateCut(event);
    
  } else if (currentScreen == SETUP_SCREEN) {
    uiUpdateSetup(event);
  }
}

void uiInit() {
  currentScreen = HOME_SCREEN;
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
      mprintf(PSTR("Encoder: %d\r\n"), getEncoderPosition());
    } else {
      mprintf(PSTR("Got event: %d\r\n"), event);
    }

    uiUpdate(event);    
  }
  
}




