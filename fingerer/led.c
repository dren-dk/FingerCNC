#include "led.h"

#include "light_ws2812.h"


#define LEDS 1

void ledIdle() {
  struct cRGBW leds[LEDS];
  leds[0].r = 0;
  leds[0].g = 0;
  leds[0].b = 60;
  leds[0].w = 0;
  ws2812_setleds_rgbw(leds,LEDS);
}

void ledReadyToCut() {
  struct cRGBW leds[LEDS];
  leds[0].r = 0;
  leds[0].g = 100;
  leds[0].b = 0;
  leds[0].w = 0;
  ws2812_setleds_rgbw(leds,LEDS);
}

void ledRunning() {
  struct cRGBW leds[LEDS];
  leds[0].r = 255;
  leds[0].g = 0;
  leds[0].b = 0;
  leds[0].w = 0;
  ws2812_setleds_rgbw(leds,LEDS);
}

void ledRunningError() {
  struct cRGBW leds[LEDS];
  leds[0].r = 255;
  leds[0].g = 0;
  leds[0].b = 0;
  leds[0].w = 55;
  ws2812_setleds_rgbw(leds,LEDS);
}
