/*
 * xconsoleio.h
 *
 * Created: 2023/04/02 20:29:21
 *  Author: 46nori
 */ 


#ifndef XCONSOLEIO_H_
#define XCONSOLEIO_H_

#include "usart.h"

extern int x_printf(const char *format, ...);
extern int x_puts(const char *s);
extern char *x_gets_s(char *buffer, size_t size);
extern int x_putchar(int c);
extern int x_getchar(void);
extern int x_getchar_tout(int32_t us);

#endif /* XCONSOLEIO_H_ */