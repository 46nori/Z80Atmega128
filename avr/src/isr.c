/*
 * isr.c
 *
 * Created: 2023/05/01 11:29:23
 *  Author: 46nori
 */ 
#include "isr.h"
#include "interrupt.h"
#include "usart.h"
#include "z80io.h"
#include "xconsoleio.h"
#include "emuldev/emuldev.h"

//
// External Interrupt
//
void ExtInt_Init(void) {
	// External interrupt
	EICRA = 0b10101010;				// Falling edge sense
	EICRB = 0b10101010;				// Falling edge sense
	//EIFR  = _BV(INTF0)|_BV(INTF1)|_BV(INTF4);
	EIMSK = _BV(INT0)|_BV(INT1)|_BV(INT4);	// Enable INT0,1 and 4
}

///////////////////////////////////////////////////////////////////
// INT0 : Z80 IN instruction(I/O READ) handler
///////////////////////////////////////////////////////////////////
ISR(INT0_vect) {
	// Emulate device and return port data to Z80
	PORTA = (*InHandler[PINF % PORT_MAX])();
	DDRA  = 0xff;					// Set Port A output

	// Clear /WAIT (instead of Z80_CLRWAIT())
	CLR_BIT(PORTD, PORTD5);			// Inactivate Z80 /WAIT
	asm("NOP");						// Ensure data latch before PortA High-Z
	PORTA = 0xff;					// Set PortA input and High-Z
	DDRA  = 0x00;
	SET_BIT(PORTD, PORTD5);

//	CLR_BIT(PORTE, PORTE5);			// DEBUG: BLUE LED ON PE5
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
	// Clear /WAIT (instead of Z80_CLRWAIT())
	Z80_CLRWAIT();

//	CLR_BIT(PORTE, PORTE6);			// DEBUG: YELLOW LED ON PE6
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
	CLR_BIT(PORTD, PORTD5);			// Inactivate Z80 /WAIT
	asm("NOP");						// Ensure data latch before PortA High-Z
	PORTA = 0xff;					// Set PortA input and High-Z
	DDRA  = 0x00;
	SET_BIT(PORTD, PORTD5);

//	CLR_BIT(PORTE, PORTE7);			// DEBUG: RED LED ON PE7
}

///////////////////////////////////////////////////////////////////
// USART1 RX handler
///////////////////////////////////////////////////////////////////
ISR(USART1_RX_vect)
{
	EnqueueRX1_NotifyZ80();
}

///////////////////////////////////////////////////////////////////
// Timer0 handler
///////////////////////////////////////////////////////////////////
static void DebugTwinkleLED(void);
ISR(TIMER0_COMP_vect) {
	Transmit_TX1_Buf();
//	DebugTwinkleLED();	
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
