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
#include <avr/eeprom.h>
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
	if (bit_is_clear(PINE, PORTE3)) {
		// Load 62K CP/M BIOS from EEPROM if DIPSW4 is ON.
		const uint16_t *src = 0;
		eeprom_busy_wait();
		uint8_t *dst = (uint8_t *)eeprom_read_word(src++);
		eeprom_busy_wait();
		size_t len = eeprom_read_word(src++);
		load_eeprom_extmem(dst, (const uint8_t *)src, len);
		Z80_RESET_GO(dst);
		x_puts("\n=== CP/M mode ===");
		x_printf("BIOS: 0x%04x - 0x%04x\n", dst, dst + len - 1);
	}
	monitor();
}
