#include "lcd.h"

//#include "../u8g2/csrc/u8x8.h"

#include <avr/io.h>
#include <util/delay_basic.h>

#include "gpio.h"

/*
 The GPIO pins used for the lcd are:
 
| LCD-CS    | PH1 | D16     | EXP1-4  |
| LCD-Data  | PH0 | D17     | EXP1-3  |
| LCD-Clock | PA1 | D23     | EXP1-5  |

*/

// This is the hardware "driver" that connects the u8g2 library to the actual hardware needed
uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch(msg) {
        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            DDRH |= _BV(PH0) | _BV(PH1);
            DDRA |= _BV(PA1);
        break;
        
        case U8X8_MSG_GPIO_CS:         GPIO(H, 1, arg_int) break;
        case U8X8_MSG_GPIO_SPI_DATA:   GPIO(H, 0, arg_int) break;
        case U8X8_MSG_GPIO_SPI_CLOCK:  GPIO(A, 1, arg_int) break;
        
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
  This is a slightly re-written version of the u8x8_byte_4wire_sw_spi for better performance
 */
uint8_t sw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
  uint8_t takeover_edge = u8x8_GetSPIClockPhase(u8x8);
  uint8_t not_takeover_edge = 1 - takeover_edge;

  if (msg == U8X8_MSG_BYTE_SEND) {
    uint8_t* data = (uint8_t *)arg_ptr;
    while( arg_int > 0 ) {
      uint8_t b = *data;
      data++;
      arg_int--;
      for (uint8_t i = 0; i < 8; i++ ) {
	GPIO(H, 0, b & 128)

	  /*  
	if ( b & 128 ) {
	  u8x8_gpio_SetSPIData(u8x8, 1);
	} else {
	  u8x8_gpio_SetSPIData(u8x8, 0);
	}
	  */
	b <<= 1;
	
	u8x8_gpio_SetSPIClock(u8x8, not_takeover_edge);
	u8x8_gpio_SetSPIClock(u8x8, takeover_edge);
      }    
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

/*
uint8_t lcdCount = 0;

void lcdHello() {
    u8g2_ClearBuffer(&u8g2);

    u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr);
    u8g2_DrawStr(&u8g2, 32, 32, "Hello!");
     
    u8g2_DrawRFrame(&u8g2, 10, 10, 60, 40, 10);
    u8g2_DrawLine(&u8g2, lcdCount,0, 127-lcdCount,63);
    if (lcdCount++ > 127) {
        lcdCount = 0;
    }
    u8g2_DrawLine(&u8g2, 127,0, 0,63);  
    u8g2_SendBuffer(&u8g2);
}
*/
