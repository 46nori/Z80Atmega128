/*
 * isr.c
 *
 * Created: 2023/05/01 11:29:23
 *  Author: 46nori
 */ 
#include "isr.h"
#include "interrupt.h"
#include "xconsoleio.h"

#if 1
//
// Z80 IN instruction handler
//
ISR(INT0_vect) {
	SET_BYTE(PORTE, 0x7f);			// LED ON PE7
}

//
// Z80 OUT instruction handler
//
ISR(INT1_vect) {
	SET_BYTE(PORTE, 0xbf);			// LED ON PE6
}

//
// Z80 external INT handler
//
ISR(INT4_vect) {
	SET_BYTE(PORTE, 0xdf);			// LED ON PE5
}

#endif