#pragma once

#include "avr8gpio.h"

/*
 * This header defines all the board specific constants so porting to a new board
 * only requires changing this file
 */
#define BOARD_RAMPS

#ifdef BOARD_RAMPS
/*
Connections for the RAMPS based prototype

| Function  | Define         | AVR | Arduino | AKA.    |
|-----------|----------------|-----|---------|---------|
| LED       | LED            | PB7 | D13     |         |
| X-enable  | MOTOR_X_ENABLE | PD7 | D38     |         |
| X-step    | MOTOR_X_STEP   | PF0 | A0      |         |
| X-dir     | MOTOR_X_DIR    | PF1 | A1      |         |
| LCD-CS    | LCD_CS         | PH1 | D16     | EXP1-4  |
| LCD-Data  | LCD_DATA       | PH0 | D17     | EXP1-3  |
| LCD-Clock | LCD_CLOCK      | PA1 | D23     | EXP1-5  |
| WS2812    | WS2812         | PB5 | D11     | Servo 1 |
| Beeper    | BEEPER         | PC0 | D37     | EXP1-1  |
| X-Min     | LIMIT_X_MIN    | PE5 | D3      |         |
| X-Max     | LIMIT_X_MAX    | PE4 | D2      |         |
| Y-Min     | LIMIT_Y_MIN    | PJ1 | D14     |         |
| Enc-btn   | ENCODER_BUTTON | PC2 | D35     | EXP1-2  |
| Enc-a     | ENCODER_A      | PC6 | D31     | EXP2-3  |
| Enc-b     | ENCODER_B      | PC4 | D33     | EXP2-5  |
| Stop      | STOP           | PG0 | D41     | EXP2-8  |
| Debug 0   | DEBUG_0        | PG5 | D4      | Servo 4 |
| Debug 1   | DEBUG_1        | PE3 | D5      | Servo 3 |
| Debug 2   | DEBUG_2        | PH3 | D6      | Servo 2 |
*/


#define LED            GPB7
#define MOTOR_X_ENABLE GPD7
#define MOTOR_X_STEP   GPF0
#define MOTOR_X_DIR    GPF1
#define LCD_CS         GPH1
#define LCD_DATA       GPH0
#define LCD_CLOCK      GPA1
#define WS2812         GPB5
#define BEEPER         GPC0
#define LIMIT_X_MIN    GPE5
#define LIMIT_X_MAX    GPE4
#define LIMIT_Y_MIN    GPJ1
#define ENCODER_BUTTON GPC2
#define ENCODER_A      GPC6
#define ENCODER_B      GPC4
#define STOP           GPG0
#define DEBUG_0        GPG5
#define DEBUG_1        GPE3
#define DEBUG_2        GPH3


#elif BOARD_PROMICRO

#else

#error No board type specified, currently known types: BOARD_RAMPS and BOARD_PROMICRO

#endif


#ifdef ANDERS
#define PULLUP_Y_MIN
#define INVERT_Y_MIN
#endif
