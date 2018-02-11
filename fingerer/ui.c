#include <avr/pgmspace.h>
#include <avr/wdt.h> 

#include <stdio.h>

#include "ui.h"
#include "uiEdit.h"
#include "uiSetup.h"
#include "uiHome.h"
#include "uiCut.h"

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
  uiSetScreen(HOME_SCREEN);
//  uiSetScreen(SETUP_SCREEN);
//  uiSetScreen(CUT_SCREEN);
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




