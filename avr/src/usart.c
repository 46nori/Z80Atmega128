/*
 * usart.c
 *
 * Created: 2023/03/31 17:41:05
 *  Author: 46nori
 */ 
#include "usart.h"
#include <stdio.h>
#include <stdarg.h>

void USART0_Init(uint32_t baud)
{
	// Set baud rate
	uint16_t br = FOSC / 16 / (baud - 1);
	UBRR0H = (uint8_t)((br>>8) & 0x0f);	// High byte
	UBRR0L = (uint8_t)(br & 0xff);		// Low byte
	// Set frame settings : async, 8bit, non-parity, 1 stop bit
	UCSR0C = _BV(UCSZ01)|_BV(UCSZ00);
	// Enable RX1 and TX1
	UCSR0B = _BV(RXEN0)|_BV(TXEN0);
}

void USART1_Init(uint32_t baud)
{
	// Set baud rate
	uint16_t br = FOSC / 16 / (baud - 1);
	UBRR1H = (uint8_t)((br>>8) & 0x0f);	// High byte
	UBRR1L = (uint8_t)(br & 0xff);		// Low byte
	// Set frame settings : async, 8bit, non-parity, 1 stop bit
	UCSR1C = _BV(UCSZ11)|_BV(UCSZ10);
	// Enable RX1 and TX1
	UCSR1B = _BV(RXEN1)|_BV(TXEN1);
}

void USART0_Transmit(uint8_t data)
{
	while (!(UCSR0A & _BV(UDRE0)));
	UDR0 = data;
}

void USART1_Transmit(uint8_t data)
{
	while (!(UCSR1A & _BV(UDRE1)));
	UDR1 = data;
}

uint8_t USART0_Receive(void)
{
	while (!(UCSR0A & _BV(RXC0)));
	return UDR0;
}

int USART0_Receive_tout(uint32_t us)
{
	if (us == 0) {
		return USART0_Receive();
	}
	while (us--) {
		if (UCSR0A & _BV(RXC0)) {
			return UDR0;
		}
		delay_us(1);
	}	
	return EOF;
}

uint8_t USART1_Receive(void)
{
	while (!(UCSR1A & _BV(RXC1)));
	return UDR1;
}

//////////////////////////////////
// USART0 serial I/O
//////////////////////////////////
int x_printf(const char *format, ...) {
	char buffer[128];
	int size;
	va_list argptr;
	va_start(argptr, format);
	size = vsnprintf(buffer, sizeof(buffer), format, argptr);
	va_end(argptr);
	for (int i = 0; i < size; i++) {
		if (buffer[i] == '\n') {
			x_putchar('\r');	
		}
		x_putchar(buffer[i]);
	}
	return size;
}

int x_puts(const char *s) {
	int st;
	while (*s) {
		st = x_putchar(*s);
		if (st < 0) {
			return st;
		}
		++s;
	}
	st = x_putchar('\r');
	st = x_putchar('\n');
	return st;
}

char *x_gets(char *buffer) {
	int c;
	char *p = buffer;
	while(1) {
		c = x_getchar();
		if (c == '\r' || c == '\n') {
			*p = '\0';
			x_putchar('\r');
			x_putchar('\n');
			break;
		} else if (c == EOF) {
			*p = '\0';
			buffer = NULL;
			break;
		} else if (0x20 <= c && c < 0x7e) {
			*p++ = c;
			x_putchar(c);
		} else if (c == 0x08) { // BS
			x_putchar(' ');
			x_putchar(c);
			if (p != buffer) {
				--p;
			}
			*p = '\0';
		}
	}
	return buffer;
}

int x_putchar(int c) {
	if (c != EOF) {
		USART0_Transmit(c);
	}
	return c;
}

int x_getchar(void) {
	return x_getchar_tout(0);
}

int x_getchar_tout(int32_t us) {
	int c = EOF;
	if (us == 0) {
		c = USART0_Receive();
	} else {
		c = USART0_Receive_tout(us);
	}
	return c;
}