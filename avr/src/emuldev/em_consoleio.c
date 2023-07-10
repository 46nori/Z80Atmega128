/*
 * em_consoleio.c
 *
 * Created: 2023/06/28 22:30:09
 *  Author: 46nori
 */ 
#include "em_consoleio.h"
#include "xconsoleio.h"
#include "z80io.h"

//=================================================================
// Console I/O emulated device
//=================================================================
#define RX1_BUF_SIZE	8
#define TX1_BUF_SIZE	128
static char rx1_buf[RX1_BUF_SIZE];
static char tx1_buf[TX1_BUF_SIZE];
static ConsoleBuffer cb_rx1;
static ConsoleBuffer cb_tx1;
static volatile uint8_t z80_int_num_rx1;
static volatile uint8_t z80_int_num_tx1;

///////////////////////////////////////////////////////////////////
// Initialize emulated device
///////////////////////////////////////////////////////////////////
void init_em_console(void)
{
	// Initialize console buffer
	initConsoleBuffer(&cb_rx1, rx1_buf, RX1_BUF_SIZE);
	initConsoleBuffer(&cb_tx1, tx1_buf, TX1_BUF_SIZE);

	// Disable Z80 interrupt at receiving RX1 by default.
	z80_int_num_rx1 = 128;
	z80_int_num_tx1 = 128;
}

///////////////////////////////////////////////////////////////////
// Receive characters and queue them to RX1 buffer
// (called from the UART1 RX ISR)
///////////////////////////////////////////////////////////////////
void Enqueue_RX1_Buf()
{
	while (UCSR1A & _BV(RXC1)) {
		if (x_enqueue(&cb_rx1, UDR1)) {
			// Notify Z80 if interrupt setting is enable
			if (z80_int_num_rx1 < 128) {
				// CAUTION: vector is NOT interrupt number(0-127)
				Z80_EXTINT_low(z80_int_num_rx1 << 1);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////
// Dequeue a character from TX1 buffer and transmit it
// (called from the periodic time ISR)
///////////////////////////////////////////////////////////////////
void Transmit_TX1_Buf(void)
{
	bool is_sent = false;
	char data;
	while ((data = x_dequeue(&cb_tx1)) != '\0') {
		USART1_Transmit(data);
		is_sent = true;
	}
	if (is_sent && z80_int_num_rx1 < 128) {
		// CAUTION: vector is NOT interrupt number(0-127)
		Z80_EXTINT_low(z80_int_num_tx1 << 1);
	}
}

///////////////////////////////////////////////////////////////////
// Condole device emulation
///////////////////////////////////////////////////////////////////
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// CAUTION: Followings are PROHIBITED here
//  * XMEM external SRAM R/W
//  * invoke /BUSREQ
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///////////////////////////////////////////////////////////////////
// Console Input
///////////////////////////////////////////////////////////////////
uint8_t IN_00_CONIN()
{
	// Get a character from RX1 console input buffer
	char data = x_dequeue(&cb_rx1);
	return data;
}

uint8_t IN_01_CONIN_GetStatus()
{
	// Get remaining characters in console input buffer
	return cb_rx1.count & 0xff;
}

uint8_t IN_02_CONIN_GetBufferSize()
{
	// Get RX1 buffer size
	return RX1_BUF_SIZE;
}
void OUT_02_CONIN_Flush(uint8_t data)
{
	x_flush(&cb_rx1);
}

void OUT_03_CONIN_SetIntLevel(uint8_t data)
{
	z80_int_num_rx1 = data;
}

uint8_t IN_03_CONIN_GetIntLevel()
{
	return z80_int_num_rx1;
}

///////////////////////////////////////////////////////////////////
// Console Output
///////////////////////////////////////////////////////////////////
void OUT_05_CONOUT(uint8_t data)
{
	// set character TX1 console buffer
	x_enqueue(&cb_tx1, data);
}

uint8_t	IN_06_CONOUT_GetStatus()
{
	// Get remaining characters in console output buffer 
	return cb_tx1.count & 0xff;	
}

uint8_t IN_07_CONOUT_GetBufferSize()
{
	return TX1_BUF_SIZE;
}

void OUT_07_CONOUT_Flush(uint8_t data)
{
	x_flush(&cb_tx1);
}

uint8_t	IN_08_CONOUT_GetIntLevel()
{
	return z80_int_num_tx1;	
}

void OUT_08_CONOUT_SetIntLevel(uint8_t data)
{
	z80_int_num_tx1 = data;
}