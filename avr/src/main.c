/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#include <asf.h>
#include "usart.h"
#include "monitor.h"

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */

	board_init();
	USART0_Init(9600);
	USART1_Init(9600);


	/* Insert application code here, after the board has been initialized. */
	// Test
	monitor();
}

#if 0
//
// Attach external memory and make Z80 suspend
// Caution: Do NOT call at the interrupt handler
//          for Z80 (/IN0, /INT1, /INT4)
//
void ExtMemory_attach()
{
	Z80_BUSREQ(1);				// Acquire memory bus
	SET_BIT(MCUCR, SRE);		// Enable XMEM
}

//
// Detach external memory and make Z80 being active
//
void ExtMemory_detach()
{
	CLR_BIT(MCUCR,  SRE);		// Disable XMEM
	SET_BYTE(DDRA,  0x00);		// Set as input
	SET_BYTE(PORTA, 0xff);		// Set Hi-Z
	SET_BYTE(DDRC,  0xff);		// Set PORTC as output
	Z80_BUSREQ(0);				// Release memory bus
}

enum shadow_size {
	MAP_32K = 0b00000111,
	MAP_16K = 0b00000110,
	MAP_8K  = 0b00000101,
	MAP_4K  = 0b00000100,
	MAP_2K  = 0b00000011,
	MAP_1K  = 0b00000010,
	MAP_256 = 0b00000001,
	UNMAP   = 0b00000000
	};
//
// Set shadow area to access hiding area of external memory
//
void *ExtMemory_map(enum shadow_size size)
{
	SET_BYTE(PORTC, 0x00);		// Force higher address disable
	SET_BYTE(DDRC,  0xff);		// Set PORTC as output
	SET_BYTE(XMCRB, size);		// Set shadow size
	
	uint16_t adr;
	if (size == UNMAP) {
		adr = 0x0000;
	} else if (size == MAP_256 ) {
		adr = 0xff00
	} else {
		adr = (uint16_t)0xff00 << size;
	}
	return (void *)adr;
}


void Z80_RESET() {
	ExtMemory_detach();
	// Send reset pulse
	CLR_BIT(PORTG, PORTG4)
	delay_ms(1);
	SET_BIT(PORTG, PORTG4)
}

void Z80_BUSREQ(int st) {
	if (st) {
		// Enable /BUSREQ
		CLR_BIT(PORTD, PORTD6);
		// Wait for /BUSACK
		while (!(GET_BYTE(PORTD) & _DV(PORTD7)));
	} else {
		// Disable /BUSREQ
		SET_BIT(PORTD, PORTD6);		
	}
}
#endif
