/*
 * isr.c
 *
 * Created: 2023/05/01 11:29:23
 *  Author: 46nori
 */ 
#include "isr.h"
#include "interrupt.h"
#include "xconsoleio.h"

volatile uint8_t	port_adr;
volatile uint8_t	port_dat = 0x12;

void ISR_Init(void) {
	// External interrupt
	EICRA = 0b10101010;				// Falling edge sense
	EICRB = 0b10101010;				// Falling edge sense
	//EIFR  = _BV(INTF0)|_BV(INTF1)|_BV(INTF4);
	EIMSK = _BV(INT0)|_BV(INT1)|_BV(INT4);	// Enable INT0,1 and 4

	// Timer0 interrupt
	OCR0  = 10 * F_CPU/1024000UL;	// 1024/16MHz x Count (every 10msec)
	TCCR0 = _BV(WGM01)|				// CTC mode
			_BV(CS02)|_BV(CS01)|_BV(CS00);	// start with 1/1024 pre-scaler
	TIFR |= _BV(OCF0);				// Interrupt every compare match
	TIMSK|= _BV(OCIE0);				// Enable interrupt
}

//
// Z80 IN instruction handler
//
ISR(INT0_vect) {
	CLR_BIT(PORTD, PORTD5);
	//----
	port_adr = PORTF;
	PORTF = port_dat;

	PORTE &= ~_BV(PORTE5);			// DEBUG: LED ON PE5
	//----
	OCR0  = 16;
	SET_BIT(PORTD, PORTD5);			// Clear WAIT by rising edge of /RELWAIT
}

//
// Z80 OUT instruction handler
//
ISR(INT1_vect) {
	CLR_BIT(PORTD, PORTD5);
	//----
	port_adr = PORTF;
	port_dat = PORTA;

	PORTE &= ~_BV(PORTE6);			// DEBUG: LED ON PE6
	//----
	OCR0  = 16;
	SET_BIT(PORTD, PORTD5);			// Clear WAIT by rising edge of /RELWAIT
}

//
// Z80 external INT handler
//
ISR(INT4_vect) {
	CLR_BIT(PORTD, PORTD5);
	//----

	PORTE &= ~_BV(PORTE7);			// DEBUG: LED ON PE7

	//----
	OCR0  = 16;
	SET_BIT(PORTD, PORTD5);			// Clear WAIT by rising edge of /RELWAIT
}

//
// Timer0 handler
//
ISR(TIMER0_COMP_vect) {
	static uint16_t i = 0;
	if (i < 100) {
		PORTE &= ~_BV(PORTE7);			// DEBUG: LED ON PE5
	} else if (i < 200) {
		PORTE |=  _BV(PORTE7);			// DEBUG: LED OFF PE5
	} else {
		i = 0;
	}
	++i;
}