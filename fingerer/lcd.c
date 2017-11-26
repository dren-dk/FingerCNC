#include "lcd.h"
#include <avr/io.h>
#include <util/delay_basic.h>
#include "../u8g2/csrc/u8g2.h"

/*
 The GPIO pins used for the lcd are:
 
| LCD-CS    | PH1 | D16     | EXP1-4  |
| LCD-Data  | PH0 | D17     | EXP1-3  |
| LCD-Clock | PA1 | D23     | EXP1-5  |

*/

#define GPIO(p,pin,on) if (on) { PORT ## p |= _BV(P ## p ## pin); } else { PORT ## p &=~ _BV(P ## p ## pin); } break;

u8g2_t u8g2;

uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

void wiggle(uint8_t a, uint8_t b) {
    
    u8x8_gpio_and_delay(0, U8X8_MSG_GPIO_SPI_DATA, 1, 0);
    while (a || b) {
        
        if (a) {
            u8x8_gpio_and_delay(0, U8X8_MSG_GPIO_CS, 1, 0);
            a--;
        }
        if (b) {
            u8x8_gpio_and_delay(0, U8X8_MSG_GPIO_SPI_CLOCK, 1, 0);
            b--;
        }
        u8x8_gpio_and_delay(0, U8X8_MSG_GPIO_CS, 0, 0);
        u8x8_gpio_and_delay(0, U8X8_MSG_GPIO_SPI_CLOCK, 0, 0);
    }
    u8x8_gpio_and_delay(0, U8X8_MSG_GPIO_SPI_DATA, 0, 0);
}


// This is the hardware "driver" that connects the u8g2 library to the actual hardware needed
uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch(msg) {
        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            DDRH |= _BV(PH0) | _BV(PH1);
            DDRA |= _BV(PA1);
        break;
        
        case U8X8_MSG_GPIO_CS:         GPIO(H, 1, arg_int)
        case U8X8_MSG_GPIO_SPI_DATA:   GPIO(H, 0, arg_int)
        case U8X8_MSG_GPIO_SPI_CLOCK:  GPIO(A, 1, arg_int)
        
        case U8X8_MSG_DELAY_NANO: // When we get here 750 ns has already passed, so there's no point in doing anything else
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
            wiggle(msg,arg_int);
            return 0;
    }
    return 1;
}


void lcdInit()  {
    u8g2_Setup_st7920_s_128x64_f(&u8g2, U8G2_R0, u8x8_byte_3wire_sw_spi, u8x8_gpio_and_delay); 
    u8g2_InitDisplay(&u8g2);    

    u8g2_SetPowerSave(&u8g2, 0);    
 
    lcdHello();

}

void lcdHello() {
    u8g2_ClearBuffer(&u8g2);

    u8g2_SetFont(&u8g2, u8x8_font_chroma48medium8_r);
    u8g2_DrawStr(&u8g2, 15,15,"Hello World!");
     
    u8g2_DrawLine(&u8g2, 0,0, 127,63);
    u8g2_DrawLine(&u8g2, 127,0, 0,63);  
    u8g2_SendBuffer(&u8g2);
}


