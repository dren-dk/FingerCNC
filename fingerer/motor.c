#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdio.h>

#include "motor.h"
#include "uart.h"
#include "inputs.h"
#include "events.h"

/*
| Function  | AVR | Arduino | AKA.    | int     |
|-----------|-----|---------|---------|---------|
| X-enable  | PD7 | D38     |         |         |
| X-step    | PF0 | A0      |         |         |
| X-dir     | PF1 | A1      |         |         |
*/


// Setup
int32_t minSpeed;
int32_t accelerationPerTick;  // steps / tick²
int32_t maxSpeed;       // Steps / second
uint32_t stepsPerMm;     // Steps / mm
uint32_t xAxisLength;    // mm

// current move
uint8_t moving;
uint16_t currentSpeed;         // Steps / second
int32_t currentTargetPosition; // Steps
int32_t currentPosition;       // Steps
int8_t currentDirection;       // +1 or 1

int32_t accelerateDistance; // steps taken while accelerating
int32_t plateauDistance;    // steps taken at max speed
int32_t decelerateDistance; // steps taken while breaking
uint8_t ticks;

uint8_t homed;

void motorStartMove(int32_t targetPosition);

uint8_t xHomed() {
  return homed;
}

int32_t motorXPosition() {
  return currentPosition;
}

void motorInit() {
  DDRD |= _BV(PD7);
  DDRF |= _BV(PF0);
  DDRF |= _BV(PF1);

  currentPosition = 0;
  moving = 0;

  // TODO: Get these from the configuration
  accelerationPerTick = 10;
  maxSpeed = 3000;
  minSpeed = 250;
  stepsPerMm = 200*2/2; // (full steps) * (microstepping) / (pitch)
  xAxisLength = 600;

  enableXMotor(0);

  // motorHome();
}

void setTimerSpeed() {
  uint16_t period;
  if (currentSpeed < 245) {
    period = 0xffff;
  } else {
    period = F_CPU / currentSpeed;
  }

  OCR1AH = period>>8;
  OCR1AL = period & 0xff;

  //P("p=%d\n", period);
}

void stopMove() {
  TCCR1B &=~ _BV(CS10);  // Disable timer interrupt.
  moving = 0;
  addEvent(EVENT_RUNNING);
}

void enableXMotor(uint8_t enabled) {
  if (enabled) {
    PORTD &=~ _BV(PD7);    
  } else {
    stopMove();
    PORTD |= _BV(PD7);
    homed=0;
  }
}

void step() {
  // This ordering lengthens the pulse a little, which might not be needed,
  // but it's probably not a bad idea
  PORTF |= _BV(PF0);
  currentPosition += currentDirection;
  PORTF &=~ _BV(PF0);
}

ISR(TIMER1_COMPA_vect) {
  uint8_t atMin = readXMin();
  uint8_t atMax = readXMax();

  if (homed < 2) {
    if (homed == 0) {
      // Going in the negative direction, expecting to hit xmin
      if (atMin) {
	homed = 1;
	currentPosition = 0;	
	stopMove();
	motorStartMove(stepsPerMm*10);
	return;
      }
    } else if (homed == 1) {
      // Going in the positive direction, expecting to get off xmin

      if (!atMin) {
	homed = 2;
	currentPosition = 0;
	addEvent(EVENT_X_AT_MIN | EVENT_ACTIVE);	  
	stopMove();
	return;		
      }
    }
    
  } else {

    if (currentDirection < 0) {
      if (atMin) {  
	P("Hit min limit @ %ld", currentPosition);
	addEvent(EVENT_MOTOR_ERROR);
	stopMove();
	return;
      }
      
    } else {
      if (atMax) {  
	P("Hit max limit @ %ld", currentPosition);
	addEvent(EVENT_MOTOR_ERROR);
	stopMove();
	return;      
      }      
    }
  }

  // Send a step pulse
  step();

  // Count down the distances
  if (accelerateDistance) {
    accelerateDistance--;
    if (ticks) {
      currentSpeed += ticks*accelerationPerTick;
      ticks = 0;
      if (currentSpeed > maxSpeed) {
	currentSpeed = maxSpeed;
      }
      setTimerSpeed();
    }
  } else if (plateauDistance) {
    if (!--plateauDistance) {
      ticks = 0; // The last plateau step, reset ticks to start breaking correctly
    }
  } else if (decelerateDistance) {
    if (ticks) {
      currentSpeed -= ticks*accelerationPerTick;
      ticks = 0;
      if (currentSpeed < minSpeed) {
	currentSpeed = minSpeed;
      }
      setTimerSpeed();
    }

    if (!--decelerateDistance) {      
      stopMove();

      if (homed < 2) {
	P("Ran out of patience searching for endstop %d\n", homed);	
      }
    }
  } else {
    stopMove();
  }
}


// This gets called by the input timer at 1kHz it's used for updating
// the speed of the motor and for reading limit switches and the stop
// button.
void motorPoll() {
  ticks++;  
}
	
// Calculates the distance (not time) it takes to accelerate from
// initial_rate to target_rate using the given acceleration:
int32_t estimateAccelerationDistance(int32_t initialrate,
				     int32_t targetrate,
				     int32_t acceleration) {

  int32_t time = (targetrate-initialrate)/accelerationPerTick;
  // Acceleration is in steps / ms² -> time is in ms
  //  P("ead: ir=%ld tr=%ld a=%ld t=%ld\n", initialrate, targetrate, acceleration, time);
  return (initialrate*time)/1000 + acceleration*(time*time)/2000;
}

// This function gives you the point at which you must start braking
// (at the rate of -acceleration) if
// you started at speed initial_rate and accelerated until this point
// and want to end at the final_rate after
// a total travel of distance.
// This can be used to compute the intersection point between acceleration and
// deceleration in the cases where the trapezoid has no plateau
// (i.e. never reaches maximum speed)
//
	/*                          + <- some maximum rate we don't care about
	                           /|\
	                          / | \
	                         /  |  + <- final_rate
	                        /   |  |
	       initial_rate -> +----+--+
	                            ^ ^
	                            | |
	        intersection_distance distance */
int32_t intersectionDistance(int32_t initialrate,
			     int32_t finalrate,
			     int32_t accelerationPerTick,
			     int32_t distance) {
  int32_t acceleration = accelerationPerTick*1000;
  
  return (2*acceleration*distance -
	  (initialrate*initialrate) +
	  (finalrate*finalrate))
         / (4*acceleration);
}	

void motorMoveTo(int32_t targetPositionum) {
  motorStartMove(stepsPerMm*targetPositionum / 1000);
}

void motorStartMove(int32_t targetPosition) {
  if (moving) {
    L("Error: Cannot start move while moving");
    return;
  }
  
  currentTargetPosition = targetPosition;

  int32_t length;
  if (targetPosition < 0) {
    length = stepsPerMm*xAxisLength;
    length = -length;
    
  } else {
    length = targetPosition-currentPosition;
  }

  currentDirection = length > 0 ? 1 : -1;
  
  // Set the direction bit. and ensure that length is positive
  if (length > 0) {
    PORTF |= _BV(PF1);
  } else {
    PORTF &=~ _BV(PF1);
    length = -length;
  }

  
  currentSpeed = minSpeed;

  accelerateDistance = estimateAccelerationDistance(minSpeed,
						    maxSpeed,
						    accelerationPerTick);
  
  decelerateDistance = estimateAccelerationDistance(maxSpeed,
						    minSpeed,
						    -accelerationPerTick);
  P("Moving from %ld to %ld len=%ld dir=%d ad=%ld dd=%ld\r\n",
    currentPosition, targetPosition, length, currentDirection,
    accelerateDistance, decelerateDistance);
  
  if (accelerateDistance < 0) {
    accelerateDistance = 0;
  }
  if (decelerateDistance < 0) {
    decelerateDistance = 0;
  }
 
  plateauDistance = length-accelerateDistance-decelerateDistance;
  if (plateauDistance < 0) {
    accelerateDistance = intersectionDistance(minSpeed, minSpeed,
					      accelerationPerTick, length);
    if (accelerateDistance < 0) {
      accelerateDistance = 0;
    } else if (accelerateDistance > length) {
      accelerateDistance = length;
    }
    
    decelerateDistance = length-accelerateDistance;
    plateauDistance = 0;
  }

  P("l=%ld a=%ld p=%ld b=%ld min=%ld max=%ld a=%ld\n",
    length, accelerateDistance, plateauDistance, decelerateDistance,
    minSpeed, maxSpeed, accelerationPerTick);


  // Set up timer 1 in CTC mode to allow OCR1A to control period
  // clock source: 16MHz no pre-scaler => min speed is 244 steps / second
  // The compa interrupt will be fired for each compare.
  TCCR1A = 0;
  TCCR1B = _BV(WGM12);
  TIMSK1 = _BV(OCIE1A);

  setTimerSpeed();

  // Switch on the timer
  TCCR1B |= _BV(CS10);  
  moving = 1;
  addEvent(EVENT_RUNNING | EVENT_ACTIVE);
  ticks = 0;
}

void motorHome() {
  enableXMotor(1);
  homed = 0;
  motorStartMove(-1);
}

uint8_t motorMoving() {
  return moving;
}
