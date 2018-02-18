# FingerCNC

One dimensional CNC machine controller for making finger (aka. box aka. comb) joints on a table saw.
https://en.wikipedia.org/wiki/Finger_joint

## The Goal

I want to be able to make fingerjoints on my table saw, using
Matthias Wandels box joint jig as inspiration, but with a micro controller
doing the calculations rather than messing about with hand-carved gears.

## The Electronics

To make purcashing easy I have chosen the plunk down the money for kit with:
* RAMPS 1.4  http://reprap.org/wiki/RAMPS_1.4
* Mega2560
* ST7920 128x64 pixel LCD  http://reprap.org/mediawiki/images/5/51/RRD_FULL_GRAPHIC_SMART_CONTROLER_SCHEMATIC.pdf
* 5x A4988 stepper drivers

This kit is crazy overkill, but it's also crazy cheap as it's mass-produced for 3D printers, so it's faster and cheaper to source all the parts with such a kit rather than go out and build a dedicated controller with only the bare minimum parts needed, someone dedicated enough to mass-production could probably optimize away most of the parts of this kit and use a much smaller MCU, but done beats perfect, so kit it is:

https://www.ebay.com/itm/3D-Printer-Kit-RAMPS-1-4-Mega2560-12864-LCD-Controller-A4988-for-Arduino-Reprap-/331599060436

To clearly indicate state (moving/free to cut) I'll use a strip of RGBW LEDS hooked to PB5.

I'll use the encoder to control all aspects of the UI.

## The Mechanics

I purchased a TR8*2 trapezoidal lead screw with an anti-backlash nut and a stepper motor from robotdigg, but any screw would probably do.

## Pinout

These are the signals used by the firmware, some of which I have traced through 4 schematics to end up with the real AVR pin name, annoying how everybody who makes a board wants to rename every signal to something new.

Reference: https://www.arduino.cc/en/Hacking/PinMapping2560

The AKA. column refers to the name of the signal on the smart adaptor or the RAMPS connector

The define column refers to the constants defined in the board.h file these are then used throughout the code in stead of
refering directly to the pin names, this way it's possible to port the code to other boards without going through every single
source file looking for GPIO references.

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

## The serial port

The serial port accessible via the USB runs at 1 Mbps, because that's the highest baud rate
possible at 16 MHz and there's no baud-rate error, unlike the more standard 115200.


## Feature numbering

This is the numbering of fingers and spaces for the two boards:

Each space is made with a number of cuts.
The home location of the blade is to the left of finger/space 0.
The first cut on board A is the left edge of space 0.
The first cut on board B is the left edge of space 1. 

```
        ____      ____      ____      ____      ____      ____ 
       |    |    |    |    |    |    |    |    |    |    |    | 
     0 |    |  1 |    |  2 |    |  3 |    |  4 |    | 5  |    | 
       |    |    |    |    |    |    |    |    |    |    |    | 
   ____|    |____|    |____|    |____|    |____|    |____|    |
  |                                                           |
  |                                                           |
  |                       Board A                             |
  |                                                           |



   ____      ____      ____      ____      ____      ____ 
  |    |    |    |    |    |    |    |    |    |    |    |
  |    |  0 |    |  1 |    |  2 |    |  3 |    |  4 |    |  6
  |    |    |    |    |    |    |    |    |    |    |    |
  |    |____|    |____|    |____|    |____|    |____|    |____
  |                                                           |
  |                        Board B                            |
  |                                                           |
  |                                                           |
```


## Menu structure

* Home screen (stop returns to this page)
 * Logo 
 * X axis motor off
 * Move Sled home
 * Click encoder to start 
* Homing screen
* Run screen
 * Space/Finger size
 * Pick A/B board (X axis moves to first cut, when selecting) 
 * Pick space (X axis moves to first cut of space, when selecting)
 * Ready to cut / Moving / Error indicator 
 * Immediate stop if X is moving and Y isn't at min -> Error
 
* Config option list
 * Blade width mm
 * Stride % (50% overlap by default)
 * Offset of home position (X axis homes with X-min and moves to home position as this is adjusted)
 * Gearing: Steps per mm (200 for 200x2 steps and TR8*2)
 * Min speed mm/s
 * Speed mm/s
 * Accel mm/s²


== Paramters

These are the parameters that are used to configure the controller
and are stored in EEPROM for persistence.

| Index | Parameter           |     type | unit  | quantum   | Editor |
|-------|---------------------|----------|-------|-----------|--------|
|     0 | Gearing    	      | uint32_t |       | steps/mm  |    int |
|     1 | Kerf  	      | uint32_t |   mm  | step size |  fixed |
|     2 | Space/Finger size   | uint32_t |   mm  | step size |  fixed |
|     3 | Offset of home      | uint32_t |   mm  | step size |  fixed |
|     4 | A/B board           | uint8_t  |       | 1         |   enum |
|     5 | Space               | uint8_t  |       | 1         |    int |
|     6 | Stride	      | uint8_t  |    %  | 1         |    int |
|     7 | Min speed	      | uint32_t | mm/s  | step size |  fixed |
|     8 | Speed		      | uint32_t | mm/s  | step size |  fixed |
|     9 | Accel		      | uint8_t	 | mm/s² | step/ms²  |  fixed |


=== Gearing (steps/mm)

The number of steps per mm, you need the following bits of information to calculate it:
* The number of physical full-steps on the motor per revolution.
* The micro stepping setting of the motor driver.
* The pitch of the lead screw.

Typical bi-polar stepper motors have 200 steps per revolution (1.8 deg/step), though 400 steps also exist.

Picking a micro stepping setting should be done so the largest possible steps can be used as accuracy and torque falls with higher micro stepping levels
The highest torque is usually achived with half-stepping, so if that gives high enough resolution, then use that.

The pitch of the screw is the length of advance per revolution, an M8 threaded rod has 1.5mm/rev and
a tr8*2 lead screw has 2 mm/rev.

As an example my machine uses a typical nema 14 motor with 200 steps/rev and I've set up half-stepping (2)
and I'm using a Trapezoidal lead screw with a pich of 2mm/rev (tr8*2), so I get a gearing of
200 steps/rev * 2 / 2 mm/rev = 200 steps/mm

Note that changing the gearing changes almost all other settings, because they are stored in steps,
so if you change this setting all other settings must be re-done.

=== Kerf (mm)

The kerf is nominally the width of the blade, but due to the inaccuracies that compound in any mechanical system,
the actual kerf ends up being larger than this, so while you might start out by setting the
blade width to the measured width of the teeth of the blade, you might find that the joints
end up being too loose.

Adjusting the kerf is done by cutting a joint and if the joint is too:
* Loose: Increase kerf.
* Tight: Decrease kerf.

=== Space/Finger size (mm)

Choose whatever size you want, as long as it's larger than the blade width.

Thinner fingers give more surface to glue, so the joing might be stronger.

=== Offset of home (mm)

Distance from the fixed endstop to the home location of the blade.

This should be set so the blade just misses the edge of the material.

=== A/B Board

Simply selects between the A and the B pattern, 0=A, 1=B, see the feature numbering section.

=== Space

Simply selects the space that's currently being cut, see the feature numbering section.


=== Stride (%)

Desired advance of the x axis between cuts in percent of the blade width, so a 3mm blade
and 50% will give a desired advance of 1.5mm.

The actual advance is calculated so it ends up fitting withe notch width, so a 5mm notch gives,
5 mm / 1.5mm = 3.33 cuts, which is impossible, so we round up to 4 cuts, which gives
5 mm / 4 cuts = 1.25 mm/cut advance.

=== Min speed (mm/s)

The mechanics, motor specifications and motor current all conspire to impose a limit on how fast
the motor can run if jumping into motion from standing still.

This speed must be found by experimentation, find the speed that causes problems and divide by at least 2

=== Speed (mm/s)

Like with Min speed the physical realities limit how fast the motor can move the x axis.

This speed must be found by experimentation, find the speed that causes problems and divide by at least 2


=== Acceleration (mm/s²)

Accelerating from min speed to speed or vice versa is done by a fixed rate of acceleration.

This value must be found by experimentation, find the value that causes problems and divide by at least 2
