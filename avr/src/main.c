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
#include "xconsoleio.h"
#include "monitor.h"
#include "emuldev/emuldev.h"
#include "z80io.h"

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */

	board_init();
	init_emuldev();		// initialize emulated devices
	sei();				// Enable interrupt

	/* Insert application code here, after the board has been initialized. */
	if (1) {
		// Load CP/M BIOS from EEPROM
		load_eeprom_extmem((uint8_t *)0xf200, 0, 2816);
		Z80_RESET_GO((uint8_t *)0xf200);
	}
	monitor();
}
