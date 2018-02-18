#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include <stdio.h>

#include "board.h"

#include "motor.h"
#include "uart.h"
#include "inputs.h"
#include "events.h"
#include "config.h"


/*
| Function  | Define         | AVR | Arduino | AKA.    |
|-----------|----------------|-----|---------|---------|
| X-enable  | MOTOR_X_ENABLE | PD7 | D38     |         |
| X-step    | MOTOR_X_STEP   | PF0 | A0      |         |
| X-dir     | MOTOR_X_DIR    | PF1 | A1      |         |
*/


// Setup
int32_t minSpeed;
int32_t accelerationPerTick;  // steps / tick²
int32_t maxSpeed;       // Steps / second
uint32_t stepsPerMm;     // Steps / mm
uint32_t xAxisLength;    // mm

// current move
volatile uint8_t moving;
uint16_t currentSpeed;         // Steps / second
int32_t currentTargetPosition; // Steps
int32_t currentPosition;       // Steps
int8_t currentDirection;       // +1 or 1

int32_t accelerateDistance; // steps taken while accelerating
int32_t plateauDistance;    // steps taken at max speed
int32_t decelerateDistance; // steps taken while breaking
uint8_t ticks;

uint8_t homed;

uint8_t asapPending =0;
uint32_t asapTargetPosition;

void motorStartMove(int32_t targetPosition, int32_t speed);

uint8_t xHomed() {
  return homed;
}

int32_t motorXPosition() {
  return currentPosition;
}

void motorInit() {
  GPOUTPUT(MOTOR_X_ENABLE);
  GPOUTPUT(MOTOR_X_STEP);
  GPOUTPUT(MOTOR_X_DIR);

  asapPending =0;
  stepsPerMm = getConfigValue(C_STEPS_PER_MM);
  accelerationPerTick = getConfigValue(C_ACCELERATION) / 1000;
  if (accelerationPerTick <= 0) {
    accelerationPerTick = 1;
  }
  maxSpeed = getConfigValue(C_SPEED);
  minSpeed = getConfigValue(C_MIN_SPEED);
  
  currentPosition = 0;
  moving = 0;

  // TODO: Get these from the configuration
  xAxisLength = 2000;

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
  
  if (asapPending) {
    asapPending = 0;
    motorStartMove(asapTargetPosition, maxSpeed);
  }
}

void enableXMotor(uint8_t enabled) {
  GPWRITE(MOTOR_X_ENABLE, enabled);
  if (!enabled) {
    homed=0;
  }
}

void step() {
  // This ordering lengthens the pulse a little, which might not be needed,
  // but it's probably not a bad idea
  GPSET(MOTOR_X_STEP);
  currentPosition += currentDirection;
  GPCLEAR(MOTOR_X_STEP);
}

uint8_t handleHoming(uint8_t atMin, uint8_t atMax) {
  if (homed == 0) {
    // Going in the positive direction, expecting to get off xmin
    if (atMax) {
      stopMove();
      P("Hit max limit @ %ld", currentPosition);
      addEvent(EVENT_MOTOR_ERROR);
    }

  } if (homed == 1) {
    // Going in the negative direction, expecting to hit xmin
    if (atMin) {
      stopMove();
      homed = 2;
      currentPosition = 0;	
      motorStartMove(stepsPerMm*600, minSpeed);
      return 1;
    }
    
  } else if (homed == 2) {
    // Going in the positive direction, expecting to get off xmin
    
    if (!atMin) {
      stopMove();
      homed = 10;
      currentPosition = 0;
      addEvent(EVENT_X_AT_MIN | EVENT_ACTIVE);	  
      return 1; 		
    }
  }

  return 0; // Keep going
}


ISR(TIMER1_COMPA_vect) {
  uint8_t atMin = readXMin();
  uint8_t atMax = readXMax();

  if (homed < 10) {
    uint8_t stop = handleHoming(atMin, atMax);
    if (stop) {
      return;
    }
    
  } else {

    if (currentDirection < 0) {
      if (atMin) {  
	stopMove();
	P("Hit min limit @ %ld", currentPosition);
	addEvent(EVENT_MOTOR_ERROR);
	return;
      }
      
    } else {
      if (atMax) {  
	stopMove();
	P("Hit max limit @ %ld", currentPosition);
	addEvent(EVENT_MOTOR_ERROR);
	return;      
      }      
    }
  }

  // Count down the distances
  if (accelerateDistance) {
    step();
    
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
    step();
    
    if (!--plateauDistance) {
      ticks = 0; // The last plateau step, reset ticks to start breaking correctly
    }
  } else if (decelerateDistance) {
    step();
    
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

      if (homed < 10) {
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
   P("ead: ir=%ld tr=%ld a=%ld t=%ld\n", initialrate, targetrate, acceleration, time);
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
  motorStartMove(stepsPerMm*targetPositionum / 1000, maxSpeed);
}

void motorStartMove(int32_t targetPosition, int32_t speed) {
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
  
  if (!length) {
    return;
  }

  currentDirection = length > 0 ? 1 : -1;
  
  // Set the direction bit. and ensure that length is positive  
  if (length > 0) {
    GPSET(MOTOR_X_DIR);
  } else {
    GPCLEAR(MOTOR_X_DIR);
    length = -length;
  }

  currentSpeed = minSpeed;
  if (speed > maxSpeed) {
    speed = maxSpeed;    
  }
  if (speed < minSpeed) {
    speed = minSpeed;    
  }  

  accelerateDistance = estimateAccelerationDistance(minSpeed,
						    speed,
						    accelerationPerTick);
  
  decelerateDistance = -estimateAccelerationDistance(speed,
						    minSpeed,
						    accelerationPerTick);
  
  P("Moving from %ld to %ld len=%ld dir=%d ad=%ld dd=%ld\r\n",
    currentPosition, targetPosition, length, currentDirection,
    accelerateDistance, decelerateDistance    
   );
  
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
  currentPosition = 0;
  motorStartMove(2*stepsPerMm, maxSpeed);
  while (motorMoving()) {
  }
  homed=1;
  motorStartMove(-1, maxSpeed);
}

uint8_t motorMoving() {
  return moving;
}

void motorToAsap(uint32_t targetPosition) {
  ATOMIC_BLOCK(ATOMIC_FORCEON) {
    if (targetPosition == 0xffffffff) {
      asapPending = 0;
    } else {
      if (moving) {
          if (asapTargetPosition != targetPosition) {
            asapTargetPosition = targetPosition;
            asapPending = 1;
          }
      } else {
        asapPending = 0;
        motorStartMove(targetPosition, maxSpeed);
      }  
    }
  }
}


