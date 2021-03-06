#include "lcd.h"

#include <util/delay_basic.h>

#include "board.h"
#include "debug.h"

/*
 The GPIO pins used for the lcd are:
| Function  | Define         | AVR | Arduino | AKA.    |
|-----------|----------------|-----|---------|---------|
| LCD-CS    | LCD_CS         | PH1 | D16     | EXP1-4  |
| LCD-Data  | LCD_DATA       | PH0 | D17     | EXP1-3  |
| LCD-Clock | LCD_CLOCK      | PA1 | D23     | EXP1-5  |
*/

const uint8_t AIR = 2;

u8g2_uint_t drawText(uint8_t x, uint8_t y, uint8_t style, char const *str) { 
  int8_t ascent = u8g2_GetAscent(&u8g2);
  int8_t descent = u8g2_GetDescent(&u8g2);
  int8_t height = ascent-descent + 2*AIR;
  
  u8g2_uint_t width = u8g2_GetStrWidth(&u8g2, str)+2*AIR;
  
  if (style & TS_CENTER) {
    x -= width/2;    
  }
  
  u8g2_SetDrawColor(&u8g2, 1);
  if (style & TS_INVERT) {
    u8g2_DrawRBox(&u8g2, x, y, width, height, 0);
    u8g2_SetDrawColor(&u8g2, 0);
  } else if (style & TS_FRAME) {
    u8g2_DrawRFrame(&u8g2, x, y, width, height, 0);    
  }

  u8g2_DrawStr(&u8g2, x+AIR, y+ascent+AIR, str);

  return width;
} 

u8g2_uint_t drawTextP(uint8_t x, uint8_t y, uint8_t style, char const *str) { 
    char tmp[50];
    strcpy_P(tmp, str);
    return drawText(x,y,style,tmp);
}


// This is the hardware "driver" that connects the u8g2 library to the actual hardware needed
uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch(msg) {
        case U8X8_MSG_GPIO_AND_DELAY_INIT:
          GPOUTPUT(LCD_CS);
          GPOUTPUT(LCD_DATA);
          GPOUTPUT(LCD_CLOCK);
        break;
        
        case U8X8_MSG_GPIO_CS:         GPWRITE(LCD_CS, arg_int); break;
        case U8X8_MSG_GPIO_SPI_DATA:   GPWRITE(LCD_DATA, arg_int); break;
        case U8X8_MSG_GPIO_SPI_CLOCK:  GPWRITE(LCD_CLOCK, arg_int); break;
        
        case U8X8_MSG_DELAY_NANO: // When we get here 750 ns has already passed, so there's no point in doing anything 
            break;
        break;    
        case U8X8_MSG_DELAY_100NANO:            
            if (arg_int < 7) {
                break;
            }
            _delay_loop_1(2);
            if (arg_int < 10) {
                break;
            }
            while (1) {
                if (arg_int <= 10) {
                    break;
                }
                _delay_loop_2(4);                
                arg_int -= 10;
            }
        break;
        case U8X8_MSG_DELAY_10MICRO:
            while (arg_int--) {
                _delay_loop_2(40);
            }
        break;
        case U8X8_MSG_DELAY_MILLI:
            while (arg_int--) {
                _delay_loop_2(4000);
            }
        break;

        default:
            return 0;
    }
    return 1;
}

/*
  This is a slightly re-written version of the u8x8_byte_4wire_sw_spi function for better performance

  u8x8_byte_4wire_sw_spi is really slow at 352 ms per full screen update (23 kb/s)
  this implementation is slightly faster at 28 ms per update (290kb/s).
  still not as fast as hw spi which ought to be able to 8 Mb/s
 */
uint8_t sw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
  if (msg == U8X8_MSG_BYTE_SEND) {
    uint8_t* data = (uint8_t *)arg_ptr;
    uint8_t clockSet = GPPORT(LCD_CLOCK);
    uint8_t clockClear = clockSet & ~ GPBV(LCD_CLOCK);

    while (arg_int > 0) {
      uint8_t b = *data;
      data++;
      arg_int--;
      uint8_t i = 8;
      do {
	GPPORT(LCD_CLOCK) = clockClear;
	GPWRITE(LCD_DATA, b & 128)
	b <<= 1;
	GPPORT(LCD_CLOCK) = clockSet;
      } while (--i);   
    }

  } else if (msg == U8X8_MSG_BYTE_INIT) {
    u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
    u8x8_gpio_SetSPIClock(u8x8, u8x8_GetSPIClockPhase(u8x8));

  } else if (msg == U8X8_MSG_BYTE_SET_DC) {
    u8x8_gpio_SetDC(u8x8, arg_int);

  } else if (msg == U8X8_MSG_BYTE_START_TRANSFER) {
    u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_enable_level);  
    u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->post_chip_enable_wait_ns, NULL);

  } else if (msg == U8X8_MSG_BYTE_END_TRANSFER) {
    u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->pre_chip_disable_wait_ns, NULL);
    u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);

  } else {
    return 0;
  }
  return 1;
}

void lcdInit()  {
  //u8g2_Setup_st7920_s_128x64_f(&u8g2, U8G2_R0, u8x8_byte_4wire_sw_spi, u8x8_gpio_and_delay);
  u8g2_Setup_st7920_s_128x64_f(&u8g2, U8G2_R0, sw_spi, u8x8_gpio_and_delay); 
  u8g2_InitDisplay(&u8g2);    
  u8g2_SetPowerSave(&u8g2, 0);    
}
