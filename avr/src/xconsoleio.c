/*
 * xconsoleio.c
 *
 * Created: 2023/04/02 20:29:04
 *  Author: 46nori
 */ 

#include <stdio.h>
#include <stdarg.h>
#include "xconsoleio.h"

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

char *x_gets_s(char *buffer, size_t size) {
	if (size == 0) {
		buffer[0] = '\0';
		return NULL;
	}
	
	int c, is_done = 0;
	char *p = buffer;
	while(!is_done) {
		c = x_getchar();
		switch (c) {
		case '\r':
		case '\n':
			*p = '\0';
			x_putchar('\r');
			x_putchar('\n');
			is_done = 1;
			break;
		case '\b': // BS
			if (p != buffer) {
				--p;
				x_putchar(c);
				x_putchar(' ');
				x_putchar(c);
			}
			*p = '\0';
			break;;
		case EOF:
			*p = '\0';
			is_done = 1;
			break;
		default:
			if (p - buffer < size) {
				if (0x20 <= c && c < 0x7e) {
					*p++ = c;
					x_putchar(c);
				}
			}
			break;
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