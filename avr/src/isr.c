/*
 * isr.c
 *
 * Created: 2023/05/01 11:29:23
 *  Author: 46nori
 */ 
#include "isr.h"
#include "interrupt.h"
#include "xconsoleio.h"

//
// Z80 IN instruction handler
//
ISR(INT0_vect) {
	x_puts("INT 0");
}

//
// Z80 OUT instruction handler
//
ISR(INT1_vect) {
	x_puts("INT 1");
}

//
// Z80 external INT handler
//
ISR(INT4_vect) {
	x_puts("INT 4");
}

