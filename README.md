# FingerCNC

One dimensional CNC machine controller for making finger (aka. box aka. comb) joints on a table saw.
https://en.wikipedia.org/wiki/Finger_joint

## The Goal

I want to be able to make fingerjoints on my table saw, using
Matthias Wandels box joint jig as inspiration, but with a micro controller
doing the calculations rather than messing about with gears.

## The Electronics

To make purcashing easy I have chosen the plunk down the money for kit with:
* RAMPS 1.4  http://reprap.org/wiki/RAMPS_1.4
* Mega2560
* 12864 LCD  http://reprap.org/mediawiki/images/5/51/RRD_FULL_GRAPHIC_SMART_CONTROLER_SCHEMATIC.pdf
* 5x A4988 stepper drivers

This kit is crazy overkill, but it's also crazy cheap as it's mass-produced for 3D printers, so it's faster and cheaper to source all the parts with such a kit rather than go out and build a dedicated controller with .

https://www.ebay.com/itm/3D-Printer-Kit-RAMPS-1-4-Mega2560-12864-LCD-Controller-A4988-for-Arduino-Reprap-/331599060436

## The Mechanics

I purchased a TR8*2 trapezoidal lead screw with an anti-backlash nut and a stepper motor from robotdigg, but any screw would probably do.

## Pinout

These are the signals used by the firmware, some of which I have traced through 4 schematics to end up with the real AVR pin name, annoying how everybody who makes a board wants to rename every signal to something new.

Reference: https://www.arduino.cc/en/Hacking/PinMapping2560

The AKA. column refers to the name of the signal on the smart adaptor or the RAMPS connector

| Function  | AVR | Arduino | AKA.    |
|-----------|-----|---------|---------|
| LED       | PB7 | D13     |         | 
| X-enable  | PD7 | D38     |         |
| X-step    | PF0 | A0      |         |
| X-dir     | PF1 | A1      |         |
| X-Min     | PE5 | D3      |         |
| X-Max     | PE4 | D2      |         |
| Y-Min     | PJ1 | D14     |         |
| LCD-CS    | PH1 | D16     | EXP1-4  |
| LCD-Data  | PH0 | D17     | EXP1-3  |
| LCD-Clock | PA1 | D23     | EXP1-5  |
| Beeper    | PC0 | D37     | EXP1-1  |
| Enc-btn   | PC2 | D35     | EXP1-2  |
| Enc-a     | PC6 | D31     | EXP2-3  |
| Enc-b     | PC4 | D33     | EXP2-5  |
| Stop      | PG0 | D41     | EXP2-8  |
| ws2812    | PB5 | D11     | Servo 1 |
