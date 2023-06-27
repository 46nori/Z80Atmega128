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
#include "sdcard.h"

#define RX1_BUF_SIZE	32
#define TX1_BUF_SIZE	32
static char rx1_buf[RX1_BUF_SIZE];
static char tx1_buf[TX1_BUF_SIZE];
ConsoleBuffer cb_rx1;
ConsoleBuffer cb_tx1;

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */

	board_init();
	
	// Init buffered I/O
    initConsoleBuffer(&cb_rx1, rx1_buf, RX1_BUF_SIZE);
    initConsoleBuffer(&cb_tx1, tx1_buf, TX1_BUF_SIZE);
	
	// Enable interrupt
	sei();

	/* Insert application code here, after the board has been initialized. */
	monitor();
}
