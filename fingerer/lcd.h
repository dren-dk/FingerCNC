#pragma once

#include "../u8g2/csrc/u8g2.h"
#include "logo.h"

u8g2_t u8g2;

void lcdInit();


typedef enum {
  TS_FRAME = 1,
  TS_INVERT = 2,
  TS_CENTER = 4,
} TextStyle;


u8g2_uint_t drawText(uint8_t x, uint8_t y, uint8_t style, char const *str);
u8g2_uint_t drawTextP(uint8_t x, uint8_t y, uint8_t style, char const *str);

