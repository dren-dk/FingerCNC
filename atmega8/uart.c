/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <joerg@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.        Joerg Wunsch
 * ----------------------------------------------------------------------------
 *
 * Stdio demo, UART implementation
 *
 * $Id: uart.c,v 1.1.2.1 2005/12/28 22:35:08 joerg_wunsch Exp $
 */
#include <stdint.h>
#include <stdio.h>

#include <avr/io.h>
#include <util/delay.h>

#include "uart.h"

int uart_putchar(char c, FILE *stream){

  if (c == '\n') {
    uart_putchar('\r', stream);
  }
  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = c;
  
  return 0;
}

int uart_getchar(FILE *stream) {

  if (UCSR0A & 1<<RXC0) {
    if (UCSR0A & _BV(FE0))
      return _FDEV_EOF;
    if (UCSR0A & _BV(DOR0))
      return _FDEV_ERR;
    
    return UDR0;
  } else {
    return -1000;
  }
}

void uartInit(void) {
  UCSR0A = _BV(U2X0);   
  UBRR0H = 0;
  UBRR0L = (F_CPU / (8UL * UART_BAUD)) - 1;
  UCSR0B = _BV(TXEN0) | _BV(RXEN0); /* tx/rx enable */
  
  static FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
  static FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);
  
  stdout = &uart_output;
  stdin  = &uart_input;
}

