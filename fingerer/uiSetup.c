#include <stdio.h>

#include "uart.h"

#include "ui.h"
#include "uiSetup.h"
#include "lcd.h"
#include "config.h"
#include "inputs.h"

const uint8_t MAX_CHARS = 20;
const uint8_t MAX_LINES = 6;
const uint8_t LINE_HEIGHT = 11;

uint8_t setupTop = 0;
uint8_t setupCurrent = 0;

void uiStartSetup() {
  L("Starting Setup\n");
  setEncoderPosition(setupCurrent);
  uiSetScreen(SETUP_SCREEN);
}

void drawOption(uint8_t index, uint8_t y) {
  u8g2_SetDrawColor(&u8g2, 1); // Draw box or font in this color
  if (index == setupCurrent) {
    u8g2_DrawRBox(&u8g2, 0, y, 128, LINE_HEIGHT, 0);
    u8g2_SetDrawColor(&u8g2, 0); // Switch to off for the text       
  }
  char tmp[MAX_CHARS];

  if (index == 0) {
    strcpy_P(tmp, PSTR("Exit"));
  } else {
    ConfigParam* cp = getConfigParam(index-1);
    sprintf_P(tmp, PSTR("%S (%S)"), cp->name, cp->unit);    
  }
  
  u8g2_DrawStr(&u8g2, 2, y+LINE_HEIGHT-3, tmp);
}

void uiUpdateSetup(Event event) {
  int16_t encPos = getEncoderPosition();
  if (encPos < 0) {
    encPos = CONFIGS_USED-1;
  } else if (encPos > CONFIGS_USED-1) {
    encPos = 0;
  }
  setEncoderPosition(setupCurrent = encPos);

  if (event == (EVENT_ENC_BTN | EVENT_ACTIVE)) {
    uiStartEdit(setupCurrent-1);
    return;
  }

  int8_t scroll = setupCurrent-setupTop;
  if (scroll < 0) {
    setupTop += scroll;
  } else if (scroll >= MAX_LINES) {
    setupTop += scroll-MAX_LINES+1;
  } 
  
  u8g2_ClearBuffer(&u8g2); 
  u8g2_SetFont(&u8g2, u8g2_font_6x12_te);

   
  uint8_t setupBottom = setupTop + MAX_LINES;
  uint8_t y = 0;
  u8g2_SetFontMode(&u8g2, 0);
  for (int i=setupTop; i<setupBottom; i++) {
    drawOption(i, y);
    y += LINE_HEIGHT;
  }  

  u8g2_SendBuffer(&u8g2);  
}
