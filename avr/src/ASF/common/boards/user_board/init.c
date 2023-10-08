/**
 * \file
 *
 * \brief User board initialization template
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include <asf.h>
#include <board.h>
#include <conf_board.h>
#include "z80io.h"
#include "usart.h"
#include "timer.h"
#include "isr.h"

void board_init(void)
{
	/* This function is meant to contain board-specific initialization code
	 * for, e.g., the I/O pins. The initialization can rely on application-
	 * specific board configuration, found in conf_board.h.
	 */
	//=====================================================================
	
	// GPIO setting
	SET_BIT(SFIOR, PUD);			// Disable Pull up

	// Port A
	SET_BYTE(PORTA, 0xff);			// Set Hi-Z      / XMEM
	SET_BYTE(DDRA,  0x00);			// Set as input  / XMEM
	// Port B
	SET_BYTE(PORTB, 0b01100000);	// /NMI=H, /RESET=H
	SET_BYTE(DDRB,  0b01100000);
	// Port C
	SET_BYTE(DDRC,  0xff);			// Set as output to set XMEM shadow
	// Port D
	SET_BYTE(PORTD, 0b10011111);	// /BUSREQ=L, /CLRWAIT=L, /XINT=H
	SET_BYTE(DDRD,  0b01110000);
	// Port E
	SET_BYTE(PORTE, 0xff);			// LED OFF(PE7,6,5)
	SET_BYTE(DDRE,  0xe0);			// PE7,6,5 for output, PE3,2 for input
	// Port F
	SET_BYTE(PORTF, 0xff);			// Set Hi-Z
	SET_BYTE(DDRF,  0x00);			// Set as input
	// Port G
	SET_BYTE(PORTG, 0b00000011);	// /WR,/RD=High, ALE=Low
	SET_BYTE(DDRG,  0b00000111);	// PG0,1,2 for output/XMEM, PG3,4 for input

	// Configure wait settings of XMEM
	// DIPSW4(PE3) - SRAM wait
	if (bit_is_clear(PINE, PORTE3)) {
		ExtMem_Init(1);				// ON  : 1wait
	} else {
		ExtMem_Init(2);				// OFF : 2wait
	}
	// HALT Z80
	Z80_HALT();						// Set HALT instruction at 0x0000

	// The process up to here must be completed within 250 ms from reset.
	//=====================================================================
	SET_BYTE(PORTE, 0xff);			// LED OFF PE7,6,5

	Z80_CLRWAIT();					// Reset WAIT circuit
	Z80_BUSREQ(0);					// /BUSREQ=H

	// DIPSW2(PG4) - UART baud rate
	if (bit_is_clear(PING, PORTG4)) {
		// ON
		USART0_Init(19200);			// UART for ATmega128 Monitor
		USART1_Init(19200);			// UART for Z80 console
		Timer0_Init(1);				// Periodic timer(TIMER0_COMP) for Z80 console
	} else {
		// OFF		
		USART0_Init(9600);			// UART for ATmega128 Monitor
		USART1_Init(9600);			// UART for Z80 console
		Timer0_Init(2);				// Periodic timer(TIMER0_COMP) for Z80 console
	}
	Timer2_Init(10);				// Periodic timer(TIMER2_COMP) for SD Card

	ExtInt_Init();					// External interrupt (INT0,1,4)
}
