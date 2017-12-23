#pragma once

#include <inttypes.h>

void inputsInit();

int16_t getEncoderPosition();
void setEncoderPosition(uint16_t pos);

uint8_t readXMin();
uint8_t readXMax();
uint8_t readYMin();
