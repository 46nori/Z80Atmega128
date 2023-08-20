/*
 * xconsoleio.h
 *
 * Created: 2023/04/02 20:29:21
 *  Author: 46nori
 */ 


#ifndef XCONSOLEIO_H_
#define XCONSOLEIO_H_

#include <stdbool.h>
#include "usart.h"

typedef struct {
	char* buffer;
	int size;
	int head;
	int tail;
	int count;
} ConsoleBuffer;

extern void initConsoleBuffer(ConsoleBuffer* cb, char* buffer, int size);
extern void x_flush(ConsoleBuffer* cb);
extern int x_enqueue(ConsoleBuffer* cb, char data);
extern char x_dequeue(ConsoleBuffer* cb);

extern int x_printf(const char *format, ...);
extern int x_puts(const char *s);
extern char *x_gets_s(char *buffer, size_t size);
extern int x_putchar(int c);
extern int x_getchar(void);
extern int x_getchar_tout(int32_t us);

extern int x_printf_TX1(const char *format, ...);

#endif /* XCONSOLEIO_H_ */