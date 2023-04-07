/*
 * z80io.c
 *
 * Created: 2023/04/07 21:44:00
 *  Author: 46nori
 */ 
#include <asf.h>
#include "z80io.h"

//
// Attach external memory and make Z80 suspend
// Caution: Do NOT call at the interrupt handler
//          for Z80 (/IN0, /INT1, /INT4)
//
void ExtMemory_attach(void) {
	Z80_BUSREQ(1);				// Acquire memory bus
	SET_BIT(MCUCR, SRE);		// Enable XMEM
}

//
// Detach external memory and make Z80 being active
//
void ExtMemory_detach(void) {
	CLR_BIT(MCUCR,  SRE);		// Disable XMEM
	SET_BYTE(DDRA,  0x00);		// Set as input
	SET_BYTE(PORTA, 0xff);		// Set Hi-Z
	SET_BYTE(DDRC,  0xff);		// Set PORTC as output
	Z80_BUSREQ(0);				// Release memory bus
}

//
// Set shadow area to access hiding area of external memory
//
void *ExtMemory_map(enum shadow_size size) {
	SET_BYTE(PORTC, 0x00);		// Force higher address disable
	SET_BYTE(DDRC,  0xff);		// Set PORTC as output
	SET_BYTE(XMCRB, size);		// Set shadow size
	
	uint16_t adr;
	if (size == UNMAP) {
		adr = 0x0000;
		} else if (size == MAP_256) {
		adr = 0xff00;
		} else {
		adr = (uint16_t)0xff00 << size;
	}
	return (void *)adr;
}

void Z80_BUSREQ(int st) {
	if (st) {
		// Enable /BUSREQ
		CLR_BIT(PORTD, PORTD6);
		// Wait for /BUSACK
		while (!(GET_BYTE(PORTD) & _BV(PORTD7)));
		} else {
		// Disable /BUSREQ
		SET_BIT(PORTD, PORTD6);
	}
}
void Z80_RESET(void) {
	ExtMemory_detach();
	// Send reset pulse
	CLR_BIT(PORTG, PORTG4);
	delay_ms(1);
	SET_BIT(PORTG, PORTG4);
}

void Z80_INT_REQ(void) {
	
}