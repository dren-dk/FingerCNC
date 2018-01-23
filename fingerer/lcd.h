#pragma once

#include "../u8g2/csrc/u8g2.h"
#include "logo.h"

u8g2_t u8g2;

void lcdInit();
u8g2_uint_t drawCenteredText(uint8_t cx, uint8_t y, uint8_t highlight, char const *str);
u8g2_uint_t drawText(uint8_t x, uint8_t y, uint8_t highlight, char const *str);
u8g2_uint_t drawTextP(uint8_t x, uint8_t y, uint8_t highlight, char const *str);

