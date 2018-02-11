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

// Set the position to move to as soon as possible, if a move is already under way,
// then the move will be started as soon as the previous move has completed.
// this position is in absolute steps.
void motorToAsap(uint32_t targetPosition);
