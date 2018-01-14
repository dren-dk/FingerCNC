#pragma once

#include "config.h"

void uiInit();
void uiHandleEvents();

typedef enum {
  HOME_SCREEN,
  HOMING_SCREEN,
  CUT_SCREEN,
  SETUP_SCREEN,
  EDIT_SCREEN,
} UIScreen;

void uiSetScreen(UIScreen screen);
void uiStartSetup();
void uiStartEdit(ConfigId id);


