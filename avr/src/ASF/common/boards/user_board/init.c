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

void board_init(void)
{
	// Disable interrupt
	cli();							// Interrupt should have been disabled here

	// Port D
	SET_BYTE(PORTD, 0b10111111);	// /INT, /CLRWAIT, /BUSREQ = Low
	SET_BYTE(DDRD,  0b01110000);

	/* This function is meant to contain board-specific initialization code
	 * for, e.g., the I/O pins. The initialization can rely on application-
	 * specific board configuration, found in conf_board.h.
	 */

	// GPIO setting
	SET_BIT(SFIOR, PUD);			// Disable Pull up
	// Port A
	SET_BYTE(PORTA, 0xff);			// Set Hi-Z      / XMEM
	SET_BYTE(DDRA,  0x00);			// Set as input  / XMEM
	// Port B
	SET_BYTE(PORTB, 0b01100000);	// /RESET, /NMI = High
	SET_BYTE(DDRB,  0b01100000);
	// Port C
	SET_BYTE(DDRC,  0xff);			// Set as output to set XMEM shadow
	// Port E
	SET_BYTE(DDRE,  0x00);			// Set as input	
	// Port F
	SET_BYTE(PORTF, 0xff);			// Set Hi-Z
	SET_BYTE(DDRF,  0x00);			// Set as input
	// Port G
	SET_BYTE(PORTG, 0b00000011);	// /WR,/RD=High, ALE=Low
	SET_BYTE(DDRG,  0b00011111);	// Set as output / XMEM(PG0,1,2)

	// Suspend Z80
	Z80_BUSREQ(1);
	Z80_CLRWAIT();
	ExtMem_init();
	Z80_HALT();
}
