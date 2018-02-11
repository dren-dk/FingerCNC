#include <stdio.h>

#include "uart.h"

#include "ui.h"
#include "uiEdit.h"
#include "uiSetup.h"
#include "lcd.h"
#include "inputs.h"


ConfigId editConfigId;

const uint8_t DIGIT_Y = 40;
const uint8_t DIGIT_WIDTH = 16;

// Digit counted from least significant digit
uint8_t editDigit; 
ConfigParam* cp;

UIScreen screenAfterEdit;

void uiStartEdit(ConfigId id, UIScreen screenToReturnToAfterEdit) {
  editConfigId = id;
  screenAfterEdit = screenToReturnToAfterEdit;
  cp = getConfigParam(id);
  editDigit = 0xff;
  setEncoderPosition(1);
  P("Started editing %S\n", cp->name);
  uiSetScreen(EDIT_SCREEN);
}

void drawTitle() {
  char tmp[30];
  sprintf_P(tmp, PSTR("%S (%S)"), cp->name, cp->unit);
  u8g2_SetFont(&u8g2, u8g2_font_6x12_te);  
  drawText(63, 7, TS_CENTER, tmp);
}

uint32_t min;
uint32_t max;
int32_t digitWeight;

void startEditingDigit() {
  setEncoderPosition(0);
  // Pre-calculate the important metadata:
  digitWeight = displayToValue(cp, pow10(editDigit));
  if (!digitWeight) {
    digitWeight = 1;
  }
  
  min = displayToValue(cp, cp->min);
  max = displayToValue(cp, cp->max);
  
  P("Weight of %d = %ld\n", editDigit, digitWeight);
}

void handleEditing(Event event) {

  // Edit a digit
  static uint8_t editing = 0;
  if (editing) {
    if (getEncoderPosition()) {
      int32_t v = cp->value;
      int32_t delta = getEncoderPosition()*digitWeight;
      v += delta;

      if (v > max) {
	v = max;
      }

      if (v < min) {
	v = min;
      }

      P("delta=%d\n", delta); 
      
      cp->value = v;
      setEncoderPosition(0);
    }
    
    if (event == (EVENT_ENC_BTN | EVENT_ACTIVE)) {
      editing = 0;
      setEncoderPosition(-editDigit);
      storeConfig(cp);
      event = 0;
    }
  }

  
  // Split the value to separate digits for display
  uint8_t digitsUsed = 0;
  uint8_t digits[10];
  
  uint32_t v = valueToDisplay(cp, cp->value);
  //  P("Display value=%ld\n", v);
  while (v > 9) {
    uint32_t div = v / 10;
    digits[digitsUsed++] = v-div*10;
    v = div;
  }
  digits[digitsUsed++] = v;
  while (digitsUsed <= cp->decimals) {
    digits[digitsUsed++]  = 0;
  }
  
  // Change the digit to edit
  if (editing) {
    if (editDigit >= digitsUsed) {
      editDigit = digitsUsed-1;
      startEditingDigit();	
    }


  } else {
    int16_t sel = getEncoderPosition();
    if (sel > 1) {
      setEncoderPosition(sel = -(digitsUsed-1));
    }
    if (sel <= -digitsUsed) {
      setEncoderPosition(sel = 1);
    }
    if (sel <= 0) {
      editDigit = -sel;
      if (event == (EVENT_ENC_BTN | EVENT_ACTIVE)) {
	editing = 1;
	startEditingDigit();	
      }
    } else {
      editDigit = 0xff;

      if (event == (EVENT_ENC_BTN | EVENT_ACTIVE)) {
        if (screenAfterEdit == SETUP_SCREEN) {
          uiReturnToSetup();
        } else {
          uiSetScreen(screenAfterEdit);          
        }
	return;
      }
    }
  } 

  //  P("e=%d d=%d\n", getEncoderPosition() , editDigit);
  
  // Write the digits
  u8g2_SetFont(&u8g2, u8g2_font_courB18_tf);

  uint8_t lastDigitX = 64-DIGIT_WIDTH+(DIGIT_WIDTH*digitsUsed)/2;
  uint8_t xpos = lastDigitX;
  for (uint8_t i=0; i<digitsUsed; i++) {
    u8g2_SetDrawColor(&u8g2, 1);
    if (i == editDigit) {
      if (editing) {
	u8g2_DrawRBox(&u8g2, xpos, DIGIT_Y-18, DIGIT_WIDTH-2, 20, 0);
	u8g2_SetDrawColor(&u8g2, 0);
    
      } else {
	u8g2_DrawRFrame(&u8g2, xpos, DIGIT_Y-18, DIGIT_WIDTH-2, 20, 0);
      }
    }
    
    u8g2_DrawGlyph(&u8g2, xpos, DIGIT_Y, '0'+digits[i]);
    xpos -= DIGIT_WIDTH;
  }

  // Add decimal point
  if (cp->decimals) {
    uint8_t x = lastDigitX-cp->decimals*DIGIT_WIDTH+DIGIT_WIDTH/2;
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawGlyph(&u8g2, x, DIGIT_Y+1, '.');    
  }

  if (!editing) {
    // Add the exit button
    u8g2_SetFont(&u8g2, u8g2_font_6x12_te);  
    drawText(64, 52, editDigit==0xff ? TS_INVERT|TS_CENTER : TS_CENTER, "Exit");
  }
}

void uiUpdateEdit(Event event) {
  u8g2_ClearBuffer(&u8g2);  

  drawTitle();
  handleEditing(event);
  
  u8g2_SendBuffer(&u8g2);  
}

