#pragma once
#include <inttypes.h>
#include <avr/io.h>

void motorInit();
void enableXMotor(uint8_t enabled);
uint8_t xHomed();
void motorPoll();
void motorHome();
int32_t motorXPosition();
void motorMoveTo(int32_t targetPositionum);
uint8_t motorMoving();


