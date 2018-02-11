#include <avr/pgmspace.h>

#include "uart.h"
#include "uiCut.h"

#include "config.h"
#include "lcd.h"
#include "inputs.h"

#include "ui.h"
#include "uiEdit.h"
#include "uiSetup.h"
#include "motor.h"
#include "fingerjoints.h"


/*
Finger size: 12.123 mm
A/B Hole
Status
Config
*/

uint8_t armed = 0;
int8_t cutOpt = 0;
int32_t pos = 0;
uint8_t editingSlot = 0;
uint32_t homePos = 0;

ConfigParam* fingerWidth;

FingerJoints fj;

void initFingerJointsFromConfig() {
    initFingerJoints(&fj, fingerWidth->value, getConfigValue(C_BOARD),
                     getConfigValue(C_SPACE), getConfigValue(C_HOME_OFFSET),
                     getConfigValue(C_BLADE_WIDTH), getConfigValue(C_STRIDE));
}

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

static const char A[] PROGMEM = "A";
static const char B[] PROGMEM = "B";

void uiUpdateCut(Event event) {
  fingerWidth = getConfigParam(C_FINGER_WIDTH);
  if (event == EVENT_NONE) {
    // Init
    armed=0;
    editingSlot = 0;
    setEncoderPosition(0);    
    initFingerJointsFromConfig();
    motorToAsap(fj.currentPos);
  }

  if (armed) {
    uiCut(event);
    
  } else if (editingSlot) {
    if (event == (EVENT_ACTIVE|EVENT_ENC_BTN)) {
      editingSlot = 0;
      setEncoderPosition(cutOpt);
    } else {
      if (getEncoderPosition() < 0) {
        setEncoderPosition(0);
      }
      
      uint8_t space = getEncoderPosition();
      if (space > 100) {
        setEncoderPosition(space = 100);      
      }
      
      startSpace(&fj, space);
      motorToAsap(fj.currentPos);
    }    
    
  } else {
    cutOpt = getEncoderPosition();
    if (cutOpt > 4) {
      setEncoderPosition(cutOpt = 0);      
    }
    if (cutOpt < 0) {
      setEncoderPosition(cutOpt = 4);
    }        
    
    if (event == (EVENT_ACTIVE|EVENT_ENC_BTN)) {
      if (cutOpt == 0) {
        // slot/tab width
        uiStartEdit(C_FINGER_WIDTH, CUT_SCREEN);        
        return;
        
      } else if (cutOpt == 1) {
        // Board
        flipBoard(&fj);
        motorToAsap(fj.currentPos);
        
      } else if (cutOpt == 2) {
        setEncoderPosition(fj.space);
        editingSlot = 1;        
        
      } else if (cutOpt == 3) {
        armed = 1;
        
      } else if (cutOpt == 4) {
        uiStartSetup(CUT_SCREEN);
        return;
      }
    }    
    
    // Check if at arm location    
  }
    
  u8g2_ClearBuffer(&u8g2);
  u8g2_SetFont(&u8g2, u8g2_font_6x12_te);  

  
  uint16_t x = 1;
  
  // First line: Cut xx.xxx mm slots
  x += drawTextP(x, 1, 0, PSTR("Cut "));

  char tmp[10];
  cpValueToString(fingerWidth, tmp, 1);
  strcat_P(tmp, PSTR(" mm"));
  x += drawText(x, 1, cutOpt==0 ? TS_FRAME : 0, tmp);
  x += drawTextP(x, 1, 0, PSTR(" slots"));
  
  // Second line: Board A slot 4
  x = 1;
  x += drawTextP(x, 12, 0, PSTR("Board "));
  x += drawTextP(x, 12, cutOpt==1 ? TS_FRAME : 0, fj.board ? B : A);
  x += drawTextP(x, 12, 0, PSTR(", slot "));
  sprintf_P(tmp, PSTR("#%hhu"), fj.space);
  x += drawText(x, 12, editingSlot ? TS_INVERT : (cutOpt==2 ? TS_FRAME : 0), tmp);
   
  // Third line: Arm/disarm / Config 
  drawTextP(32, 1+11*2, TS_CENTER | (cutOpt==3 ? TS_FRAME : 0) | (armed ? TS_INVERT : 0), 
            armed ? PSTR("Armed") : PSTR("Arm"));
  drawTextP(32+63, 1+11*2, TS_CENTER | (cutOpt==4 ? TS_FRAME : 0), 
            PSTR("Config"));
  
  // Fourth line: Status
  
  u8g2_SendBuffer(&u8g2);  
}
