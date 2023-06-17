/*
 * isr.c
 *
 * Created: 2023/05/01 11:29:23
 *  Author: 46nori
 */ 
#include "isr.h"
#include "interrupt.h"
#include "xconsoleio.h"
#include "z80io.h"

//
// External Interrupt
//
void ExtInt_Init(void) {
	// External interrupt
	EICRA = 0b10101010;				// Falling edge sense
	EICRB = 0b10101010;				// Falling edge sense
	EIFR  = _BV(INTF0)|_BV(INTF1)|_BV(INTF4);
	EIMSK = _BV(INT0)|_BV(INT1)|_BV(INT4);	// Enable INT0,1 and 4
}

volatile uint8_t	port_adr;
volatile uint8_t	port_dat;

//
// Z80 IN instruction(I/O READ) handler
//
ISR(INT0_vect) {
//	CLR_BIT(PORTE, PORTE5);			// DEBUG: BLUE LED ON PE5
	port_adr = PINF;				// Check I/O address
	PORTA = 0x55;					// DUMMY return result according to designated port
	DDRA  = 0xff;					// Set PortA output
//	Z80_CLRWAIT();
	CLR_BIT(PORTD, PORTD5);			// Inactivate Z80 /WAIT
	asm("NOP");						// Ensure data latch before PortA High-Z
	PORTA = 0xff;					// Set PortA input and High-Z
	DDRA  = 0x00;
	SET_BIT(PORTD, PORTD5);
}

//
// Z80 OUT instruction(I/O WRITE) handler
//
ISR(INT1_vect) {
//	CLR_BIT(PORTE, PORTE6);			// DEBUG: YELLOW LED ON PE6
	PORTA = 0xff;
	DDRA  = 0x00;					// Set PortA input
	port_adr = PORTF;
	port_dat = PINA;
	Z80_CLRWAIT();
}

//
// Z80 external INT handler
//
ISR(INT4_vect) {
//	CLR_BIT(PORTE, PORTE7);			// DEBUG: RED LED ON PE7
	PORTF = z80_intvect;
	Z80_CLRWAIT();
}

//
// Timer0 handler
//
ISR(TIMER0_COMP_vect) {
	static uint16_t i = 0;
	if (i < 100) {
		PORTE &= ~_BV(PORTE7);			// DEBUG: RED LED ON PE5
	} else if (i < 200) {
		PORTE |=  _BV(PORTE7);			// DEBUG: RED LED OFF PE5
	} else {
		i = 0;
	}
	++i;
}