/*
 * isr.c
 *
 * Created: 2023/05/01 11:29:23
 *  Author: 46nori
 */ 
#include "isr.h"
#include "interrupt.h"
#include "z80io.h"
#include "emuldev/emuldev.h"

static void DebugTwinkleLED(void);

//
// External Interrupt
//
void ExtInt_Init(void) {
	// External interrupt
	EICRA = 0b10101010;				// Falling edge sense
	EICRB = 0b10101010;				// Falling edge sense
	// Enable INT0,1 and 4
	EIMSK = _BV(INT0)|_BV(INT1)|_BV(INT4);
}

///////////////////////////////////////////////////////////////////
// INT0 : Z80 IN instruction(I/O READ) handler
///////////////////////////////////////////////////////////////////
ISR(INT0_vect) {
	// Emulate device and return port data to Z80
	PORTA = (*InHandler[PINF % PORT_MAX])();
	DDRA  = 0xff;					// Set Port A output

	// Clear /WAIT
	// Pulse width must be >250ns (Z80 1CLK@4MHz)
	//   62.5ns(AVR 1CLK@16MHz) * 5CLK = 312.5ns
	CLR_BIT(PORTD, PORTD5);			// Inactivate Z80 /WAIT
	asm("NOP");						// 1CLK, Ensure data latch before PortA High-Z
	PORTA = 0xff;					// 1CLK, Set PortA input and High-Z
	DDRA  = 0x00;					// 1CLK
	SET_BIT(PORTD, PORTD5);			// 2CLK
}

///////////////////////////////////////////////////////////////////
// INT1 : Z80 OUT instruction(I/O WRITE) handler
///////////////////////////////////////////////////////////////////
ISR(INT1_vect) {
	// Set Port A input
	PORTA = 0xff;
	DDRA  = 0x00;
	// Emulate device with Z80 port data
	(*OutHandler[PINF % PORT_MAX])(PINA);

	// Clear /WAIT
	// Pulse width must be >250ns (Z80 1CLK@4MHz)
	//   62.5ns(AVR 1CLK@16MHz) * 5CLK = 312.5ns
	CLR_BIT(PORTD, PORTD5);			// Inactivate Z80 /WAIT
	asm("NOP");						// 1 CLK
	asm("NOP");						// 1 CLK
	asm("NOP");						// 1 CLK
	SET_BIT(PORTD, PORTD5);			// 2 CLK
}

///////////////////////////////////////////////////////////////////
// INT4 : Z80 external INT handler
///////////////////////////////////////////////////////////////////
ISR(INT4_vect) {
	// Z80 /INT = High
	Z80_EXTINT_High();
	// Z80 int vector
	PORTA = z80_int_vector;
	DDRA  = 0xff;

	// Clear /WAIT (instead of Z80_CLRWAIT())
	// Pulse width must be >250ns (Z80 1CLK@4MHz)
	//   62.5ns(AVR 1CLK@16MHz) * 5CLK = 312.5ns
	CLR_BIT(PORTD, PORTD5);			// Inactivate Z80 /WAIT
	asm("NOP");						// 1 CLK, Ensure data latch before PortA High-Z
	PORTA = 0xff;					// 1 CLK Set PortA input and High-Z
	DDRA  = 0x00;					// 1 CLK
	SET_BIT(PORTD, PORTD5);			// 2 CLK
}

///////////////////////////////////////////////////////////////////
// USART1 RX handler
///////////////////////////////////////////////////////////////////
ISR(USART1_RX_vect)
{
	Enqueue_RX1_Buf();
}

///////////////////////////////////////////////////////////////////
// Timer0 handler
///////////////////////////////////////////////////////////////////
ISR(TIMER0_COMP_vect) {
	Transmit_TX1_Buf();
}

///////////////////////////////////////////////////////////////////
// Timer2 handler
///////////////////////////////////////////////////////////////////
ISR(TIMER2_COMP_vect) {
	DebugTwinkleLED();
	em_disk_read();
	em_disk_write();
}

// Debug
static void DebugTwinkleLED(void)
{
	static uint16_t i = 0;
	if (i < 100) {
		PORTE &= ~_BV(PORTE7);			// DEBUG: RED LED ON PE7
	} else if (i < 200) {
		PORTE |=  _BV(PORTE7);			// DEBUG: RED LED OFF PE7
	} else {
		i = 0;
	}
	++i;
}
