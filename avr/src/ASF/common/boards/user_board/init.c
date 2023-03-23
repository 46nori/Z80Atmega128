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
#include <ioport.h>

void board_init(void)
{
	sysclk_init();
	board_init();
	ioport_init();
	
	delay_init(sysclk_get_cpu_hz());

	/* This function is meant to contain board-specific initialization code
	 * for, e.g., the I/O pins. The initialization can rely on application-
	 * specific board configuration, found in conf_board.h.
	 */
	ioport_set_pin_dir(Z80_RESET,   IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(Z80_INT,     IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(Z80_CLRWAIT, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(Z80_BUSRQ,   IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(Z80_BUSACK,  IOPORT_DIR_INPUT);
	ioport_set_port_dir(PORTF, IOPORT_DIR_INPUT);
	
	// Test
	while (1) {
		ioport_set_pin_level(Z80_BUSRQ, IOPORT_PIN_LEVEL_LOW);
		delay_ms(1000);
		ioport_set_pin_level(Z80_BUSRQ, IOPORT_PIN_LEVEL_HIGH);
	}

}
