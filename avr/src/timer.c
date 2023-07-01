/*
 * timer.c
 *
 * Created: 2023/06/11 21:58:15
 *  Author: miyoshi
 */ 
#include "timer.h"

void Timer0_Init(void) {
	// Timer0 interrupt
	OCR0  = 1 * F_CPU/1024000UL;	// 1024/16MHz x Count (every 1msec)
	TCCR0 = _BV(WGM01)|				// CTC mode
	_BV(CS02)|_BV(CS01)|_BV(CS00);	// start with 1/1024 pre-scaler
	TIFR |= _BV(OCF0);				// Interrupt every compare match
	TIMSK|= _BV(OCIE0);				// Enable interrupt
}