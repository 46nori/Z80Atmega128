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
	volatile long c = F_CPU;

	monitor();
	/* Insert application code here, after the board has been initialized. */
	// Test
	uint8_t i = 0;
	while (1) {
		SET_BIT(PORTD, 6);
		USART0_Transmit('0' + i++%10);
		CLR_BIT(PORTD, 6);
		USART1_Transmit('0' + i++%10);
	}

}

#if 0
void EnableSRAM()
{
	SET_BIT(MCUCR, SRE);
}

void DisableSRAM()
{
	CLR_BIT(MCUCR,  SRE);		// Disable XMEM
	SET_BYTE(DDRA,  0x00);		// Set as input
	SET_BYTE(PORTA, 0xff);		// Set Hi-Z
	SET_BYTE(DDRC,  0xff);		// Set as output

}

enum map_size	{UNMAP=0, MAP_256=1, MAP_1K=2, MAP_2K=3,
				 MAP_4K=4, MAP_8K=5, MAP_16K=6, MAP_32K=7};
void *MapSRAM(enum map_size size)
{
	if (size == UNMAP) {
		return (void *)0;
	}
	SET_BYTE(PORTC, 0x00);
	SET_BYTE(DDRC,  0xf);		// Set as output
	SET_BYTE(XMCRB, size);
	return (void *)((uint16_t)0x100 << size);
}
#endif