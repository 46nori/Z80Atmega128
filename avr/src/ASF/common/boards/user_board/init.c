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

#define SET_BIT(port, bit)   ((port) |=  _BV(bit))
#define CLR_BIT(port, bit)   ((port) &= ~_BV(bit))
#define SET_BYTE(port, byte) ((port) = (uint8_t)(byte))

void board_init(void)
{
	sysclk_init();
	delay_init(sysclk_get_cpu_hz());

	/* This function is meant to contain board-specific initialization code
	 * for, e.g., the I/O pins. The initialization can rely on application-
	 * specific board configuration, found in conf_board.h.
	 */

	// GPIO setting
	SET_BIT(SFIOR, PUD);		// Disable Pull up
	SET_BYTE(DDRA,  0x00);		// Set as input  / XMEM
	SET_BYTE(PORTA, 0xff);		// Set Hi-Z      / XMEM
	SET_BYTE(DDRB,  0x00);
	SET_BYTE(DDRC,  0xff);		// Set as output / XMEM
	SET_BYTE(DDRD,  0b01110000);
	SET_BYTE(PORTD, 0b11111111);
	SET_BYTE(DDRF,  0x00);		// Set as input
	SET_BYTE(DDRG,  0b00011111);// Set as output / XMEM(PG0,1,2)
	SET_BYTE(PORTG, 0x00010011);

	// XMEM setting
#if 0
	// SRAM access time < 100ns
	SET_BIT(MCUCR, SRE|SRW10);	// Enable XMEM, 2wait(SRW10=High)
	SET_BYTE(XMCRA, 0x00);      // All memory,  2wait(SRW11=Low)
#else
	// SRAM access time >= 100ns
	SET_BIT(MCUCR, SRE);		// Enable XMEM, 2wait(SRW10=Low)
	SET_BYTE(XMCRA, 0x02);      // All memory,  2wait(SRW11=High)
#endif

	// Test
	while (1) {
		//ioport_set_pin_level(Z80_BUSRQ,   IOPORT_PIN_LEVEL_LOW);
		SET_BIT(PORTD, 6);
		for (int i = 0; i < 5; i++) {
			delay_us(20);
		}
		//ioport_set_pin_level(Z80_BUSRQ,   IOPORT_PIN_LEVEL_HIGH);
		CLR_BIT(PORTD, 6);
		for (int i = 0; i < 5; i++) {
			delay_us(20);
		}
	}

}
