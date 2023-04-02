/*
 * xmodem.h
 *
 * Created: 2023/04/02 15:18:25
 *  Author: 46nori
 */ 

#ifndef XMODEM_H_
#define XMODEM_H_

#include <asf.h>

extern int r_xmodem(uint8_t *dst, size_t *size);
extern int s_xmodem(uint8_t *src, size_t blocks);

#endif /* XMODEM_H_ */