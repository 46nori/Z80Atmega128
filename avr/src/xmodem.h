/*
 * xmodem.h
 *
 * Created: 2023/04/02 15:18:25
 *  Author: 46nori
 */ 

#ifndef XMODEM_H_
#define XMODEM_H_

#include <stdio.h>

typedef int (*copyfunc)(unsigned char *dst, unsigned char *src, size_t size);
extern int r_xmodem(unsigned char *dst, size_t *size, copyfunc cfunc);
extern int s_xmodem(unsigned char *src, size_t blocks);

#endif /* XMODEM_H_ */
