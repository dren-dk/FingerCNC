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

| Function  | AVR | Arduino | AKA.    | int     |
|-----------|-----|---------|---------|---------|
| LED       | PB7 | D13     |         |         |
| X-enable  | PD7 | D38     |         |         |
| X-step    | PF0 | A0      |         |         |
| X-dir     | PF1 | A1      |         |         |
| LCD-CS    | PH1 | D16     | EXP1-4  |         |
| LCD-Data  | PH0 | D17     | EXP1-3  |         |
| LCD-Clock | PA1 | D23     | EXP1-5  |         |
| WS2812    | PB5 | D11     | Servo 1 |         |
| Beeper    | PC0 | D37     | EXP1-1  |         |
| X-Min     | PE5 | D3      |         | INT 5   |
| X-Max     | PE4 | D2      |         | INT 4   |
| Y-Min     | PJ1 | D14     |         | PCI 10  |
| Enc-btn   | PC2 | D35     | EXP1-2  |         |
| Enc-a     | PC6 | D31     | EXP2-3  |         |
| Enc-b     | PC4 | D33     | EXP2-5  |         |
| Stop      | PG0 | D41     | EXP2-8  |         |


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
     0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 | 11 | 
       |    |    |    |    |    |    |    |    |    |    |    | 
   ____|    |____|    |____|    |____|    |____|    |____|    |
  |                                                           |
  |                                                           |
  |                       Board A                             |
  |                                                           |



   ____      ____      ____      ____      ____      ____ 
  |    |    |    |    |    |    |    |    |    |    |    |
  |  0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 | 11
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
 * Move Sled home / click encoder to start 

* Run screen (Motor enables and homes x-axis, when at Y-min) 
 * Space/Finger size
 * Pick A/B board (X axis moves to first cut, when selecting) 
 * Pick space (X axis moves to first cut of space, when selecting)
 * Ready to cut / Moving / Error indicator 
 
 * Immediate stop if X is moving and Y isn't at min -> Error
 
* Config option list
 * Blade width mm
 * Stride % (50% overlap by default)
 * Offset of home position (X axis homes with X-min and moves to home position as this is adjusted)
 * Gearing: Steps per mm (1600 for 200x16 steps and TR8*2)
 * Min speed mm/s
 * Speed mm/s
 * Accel mm/s²
 
 
 
 
    
    