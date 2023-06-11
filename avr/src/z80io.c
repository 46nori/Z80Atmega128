/*
 * z80io.c
 *
 * Created: 2023/04/07 21:44:00
 *  Author: 46nori
 */ 
#include "z80io.h"
#include "xconsoleio.h"

//
// Initialize external memory settings
//
void ExtMem_Init(void) {
	// WAIT setting for external SRAM
#if 0
	// SRAM access time < 100ns
	SET_BIT(MCUCR, SRW10);					// 1wait(SRW10=1)
	SET_BYTE(XMCRA, _BV(SRW00));			// 1wait(SRW0x=01, SRW11=0)
#else
	// SRAM access time >= 100ns
	CLR_BIT(MCUCR, SRW10);					// 2wait(SRW10=0)
	SET_BYTE(XMCRA, _BV(SRW01)|_BV(SRW11));	// 2wait(SRW0x=10, SRW11=1)
#endif
	SET_BYTE(XMCRB, 0x00);		// Disable external memory bus-keeper
	
	SET_BYTE(PORTA, 0xff);		// Set PORTA as Hi-Z
	SET_BYTE(DDRA,  0x00);		//
	SET_BYTE(PORTC, 0x00);		// Force higher address disable
	SET_BYTE(DDRC,  0xff);		// Set PORTC as output
}

//
// Attach external memory and make Z80 suspend
// Caution: Do NOT call at the interrupt handler
//          for Z80 (/IN0, /INT1, /INT4)
//
void ExtMem_attach(void) {
	Z80_BUSREQ(1);				// Acquire memory bus
	SET_BIT(MCUCR, SRE);		// Enable XMEM
}

//
// Detach external memory and let Z80 be active
//
void ExtMem_detach(void) {
	CLR_BIT(MCUCR, SRE);		// Disable XMEM
	Z80_BUSREQ(0);				// Release memory bus
}

//
// Set shadow area to access hiding area of external memory
//
void *ExtMem_map(void) {
	SET_BYTE(XMCRB, 0x03);	// Enable shadow
	return (void *)0x2000;
}

void ExtMem_unmap(void) {
	SET_BYTE(XMCRB, 0x00);	// Disable shadow
}

//
// Z80 control
//
void Z80_BUSREQ(int st) {
	if (st) {
		// Enable /BUSREQ
		CLR_BIT(PORTD, PORTD6);
		// Wait for /BUSACK
		while (!(PIND & ~_BV(PORTD7))) {
			x_putchar('#');
		}
	} else {
		// Disable /BUSREQ
		SET_BIT(PORTD, PORTD6);
	}
}

void Z80_RESET(void) {
	ExtMem_detach();
	// Send reset pulse
	CLR_BIT(PORTB, PORTB5);
	_delay_ms(10);
	SET_BIT(PORTB, PORTB5);
}

//
//  NMI
//   /NMI required >80ns low period in Z80A
void Z80_NMI(void) {
	CLR_BIT(PORTD, PORTB6);
	asm("NOP");					// 62.5ns (=1CLK@16MHz)
	asm("NOP");					// 62.5ns (=1CLK@16MHz)
	SET_BIT(PORTD, PORTB6);	
}

//
// Clear /WAIT
//   /SET required >20ns low period in 74HC74
void Z80_CLRWAIT(void) {
	CLR_BIT(PORTD, PORTD5);
	asm("NOP");					// 62.5ns (=1CLK@16MHz)
	SET_BIT(PORTD, PORTD5);
}

//
// Set HALT instruction at 0x0000 to stay Z80 halt after reset
//
void Z80_HALT(void) {
	SET_BIT(MCUCR, SRE);		// Enable XMEM
	*(volatile uint8_t *)ExtMem_map() = 0x76;	// HALT instruction
	ExtMem_unmap();
	CLR_BIT(MCUCR, SRE);		// Disable XMEM
}