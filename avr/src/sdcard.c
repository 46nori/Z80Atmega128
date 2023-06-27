/*
 * sdcard.c
 *
 * Created: 2023/06/27 07:36:53
 *  Author: 46nori
 */ 
#include "xconsoleio.h"

#define F_CPU 16000000UL		// Workaround

#include <avr/io.h>
#include "petitfs/pff.h"
#include "petitfs/diskio.h"
#include "sdcard.h"

#define EXAMPLE_FILENAME "EXAMPLE.TXT"

/* LED0 = PD5 */
#define LEDPORT PORTE
#define LED0PIN (1 << 5)
#define LED_OFF() (LEDPORT |= LED0PIN)
#define LED_ON() (LEDPORT &= ~LED0PIN)
#define LED_IS_ON() !(LEDPORT & LED0PIN)

#define BUFFER_SIZE 10

FATFS file_system;

uint8_t write_buffer[BUFFER_SIZE] = {'H', 'e', 'l', 'l', 'o', ' ', 'S', 'D', '!', '\0'};
uint8_t read_buffer[BUFFER_SIZE]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
UINT    byte_counter              = 0;

void init_sd_card(void);

int sdcard_test(void)
{
	/* Set clock to known value, e.g. 5MHz */
//	_PROTECTED_WRITE(CLKCTRL_MCLKCTRLB, (CLKCTRL_PEN_bm | CLKCTRL_PDIV_4X_gc));

	/* Initialize card */
	init_sd_card();

	/* Set file pointer to beginning of file */
	pf_lseek(0);

	/* Write buffer */
	pf_write(write_buffer, BUFFER_SIZE, &byte_counter);
	if (byte_counter < BUFFER_SIZE) {
		x_printf("write %d bytes\n", byte_counter);
		/* End of file */
	}

	/* Finalize write */
	pf_write(0, 0, &byte_counter);

	/* Reset file pointer to beginning of file */
	pf_lseek(0);

	/* Read back the same bytes */
	pf_read(read_buffer, BUFFER_SIZE, &byte_counter);

	x_printf("read %d bytes\n", byte_counter);
	x_printf("rbuf:%s\n", read_buffer);
	x_printf("wbuf:%s\n", write_buffer);

	/* Check they're the same */
	for (int i = 0; i < byte_counter; i++) {
		if (write_buffer[i] != read_buffer[i]) {
			/* ERROR! */
			LED_ON();
			return -1;
		}
	}
	/* SUCCESS! */
	x_puts((const char*)read_buffer);
	LED_OFF();
	return 0;
}

void init_sd_card(void)
{
	DSTATUS status;
	FRESULT result;

	/* Initialize physical drive */
	do {
		status = disk_initialize();
		if (status != 0) {
			LED_ON();
		} else {
			LED_OFF();
			/* Set SPI clock faster after initialization */
			SPSR |= _BV(SPI2X);		// 1/32 With 16MHz F_CPU
		}
		/* The application will continue to try and initialize the card.
		 * If the LED is on, try taking out the SD card and putting
		 * it back in again.  After an operation has been interrupted this is
		 * sometimes necessary.
		 */
	} while (LED_IS_ON());

	/* Mount volume */
	result = pf_mount(&file_system);
	if (result != FR_OK) {
		x_puts("mount error");
		LED_ON();
	}

	/* Open file */
	result = pf_open(EXAMPLE_FILENAME);
	if (result != FR_OK) {
		x_puts("open error");
		LED_ON();
	}
}
