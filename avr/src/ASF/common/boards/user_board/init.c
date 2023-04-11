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
	sysclk_init();
	delay_init(sysclk_get_cpu_hz());

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
	// Port D
	SET_BYTE(PORTD, 0b11111111);	// /INT, /CLRWAIT, /BUSREQ = High
	SET_BYTE(DDRD,  0b01110000);
	// Port E
	SET_BYTE(DDRE,  0x00);			// Set as input	
	// Port F
	SET_BYTE(DDRF,  0x00);			// Set as input
	// Port G
	SET_BYTE(PORTG, 0b00000011);	// /WR,/RD=High, ALE=Low
	SET_BYTE(DDRG,  0b00011111);	// Set as output / XMEM(PG0,1,2)

	Z80_CLRWAIT();
	Z80_RESET();
//	Z80_BUSREQ(1);
	// Suspend Z80
//	CLR_BIT(PORTD, PORTD6);			// /BREQ = L

	ExtMemory_init();
}
