/*
 * main.c
 *
 *  Created on: 22 giu 2019
 *      Author: lorenzo
 */

#include <avr/io.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdio.h>

/* Block to select the baudrate and precalculate prescaler values */
#ifndef BAUD
#define BAUD 115200
#endif
#include <util/setbaud.h>

#define    UDRE    UDRE0
#define    UDR    UDR0
#define    RXC    RXC0
#define    RXEN    RXEN0
#define    TXEN    TXEN0
#define    UCSZ1    UCSZ01
#define    UCSZ0    UCSZ00

void uart_init(void)
{

    /* Enable UART power */
    power_usart0_enable();

    /* configure ports double mode */
    #if USE_2X
        UCSR0A |= _BV(U2X0);
    #else
        UCSR0A &= ~(_BV(U2X0));
    #endif

    /* configure the ports speed */
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

    // asynchronous, 8N1 mode
    UCSR0C = _BV(UCSZ1) | _BV(UCSZ0);


    /* Enable RX and TX */
    UCSR0B = _BV(RXEN) | _BV(TXEN);

    /* RX interrupt */
    UCSR0B |= (1<<RXCIE0);
}

ISR(USART_RX_vect)
{
    /* byte received ! */
}

int uart_putchar(char c, FILE *stream)
{
    loop_until_bit_is_set(UCSR0A, UDRE);
    UDR = c;
    return 0;
}

void uart_putstring(char *str)
{
    while (*str != 0)
    {
        uart_putchar(*str, NULL);
        str++;
    }
}

char uart_getchar(FILE *stream)
{
    loop_until_bit_is_set(UCSR0A, RXC);
    return UDR;
}

extern void frogfs_execute_test(void);  /**< the test routine */

FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
//FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);

int main(void)
{
    uint16_t i = 0;

    uart_init();             /* init and enable uart */
    stdout = &uart_output;   /* setup stdio to use uart */

    _delay_ms(500);

    if ((bit_is_set(UCSR0A, RXC)) && (UDR == 'd'))
    {   /* EEPROM dump routine selected */
        for (i = 0; i < (uint16_t)((uint16_t)E2END + (uint16_t)1U); i++)
        {
            printf_P(PSTR("%02X "), eeprom_read_byte((void*)i));
            if (i != 0 && i % 16 == 0) printf_P(PSTR("\r\n"));
        }
    }
    else
    {
        frogfs_execute_test();   /* execute tests */
    }

    while(1)
    {

    }

    return 0;    /* shall never happen */
}
