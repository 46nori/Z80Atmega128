/*
 * main.c
 *
 * Created: 2023/03/05 11:29:23
 *  Author: 46nori
 */ 
#include <asf.h>
#include <avr/eeprom.h>
#include "z80io.h"
#include "usart.h"
#include "timer.h"
#include "isr.h"
#include "xconsoleio.h"
#include "emuldev/emuldev.h"
#include "monitor.h"

int main (void)
{
	board_init();
	
	// DIPSW2(PG4) - UART baud rate
	if (bit_is_clear(PING, PORTG4)) {
		// ON
		USART0_Init(19200);			// UART0 for ATmega128 Monitor
		USART1_Init(19200);			// UART1 for Z80 console
		Timer0_Init(1);				// UART1 console out every 1ms
		} else {
		// OFF
		USART0_Init(9600);			// UART0 for ATmega128 Monitor
		USART1_Init(9600);			// UART1 for Z80 console
		Timer0_Init(2);				// UART1 console out every 2ms
	}

	Timer2_Init(10);				// R/W SD Card every 10ms
	ExtInt_Init();					// Initialize INT0,1,4
	init_emuldev();					// Initialize emulated devices

	sei();							// Enable interrupt

	// DIPSW1(PG3) - CP/M launch control
	if (bit_is_clear(PING, PORTG3)) {
		// Load 62K CP/M BIOS from EEPROM if DIPSW1 is ON.
		const uint16_t *src = 0;
		eeprom_busy_wait();
		uint8_t *dst = (uint8_t *)eeprom_read_word(src++);
		eeprom_busy_wait();
		size_t len = eeprom_read_word(src++);
		load_eeprom_extmem(dst, (const uint8_t *)src, len);
		Z80_RESET_GO(dst);			// Start CP/M
		x_puts("\n=== CP/M mode ===");
		x_printf("BIOS: 0x%04x - 0x%04x\n", dst, dst + len - 1);
	}

	monitor();						// Start Tiny Monitor
}
