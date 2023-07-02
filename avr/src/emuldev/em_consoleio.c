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
#define TX1_BUF_SIZE	16
static char rx1_buf[RX1_BUF_SIZE];
static char tx1_buf[TX1_BUF_SIZE];
static ConsoleBuffer cb_rx1;
static ConsoleBuffer cb_tx1;
static volatile uint8_t z80_int_num_rx1;

//
// Initialize emulated device
//
void init_em_console(void)
{
	// Initialize console buffer
	initConsoleBuffer(&cb_rx1, rx1_buf, RX1_BUF_SIZE);
	initConsoleBuffer(&cb_tx1, tx1_buf, TX1_BUF_SIZE);

	// Disable Z80 interrupt at receiving RX1 by default.
	z80_int_num_rx1 = 128;
}

///////////////////////////////////////////////////////////////////
// Receive characters and queue them to RX1 buffer
// (called from the UART1 RX ISR)
///////////////////////////////////////////////////////////////////
void EnqueueRX1_NotifyZ80()
{
	while (UCSR1A & _BV(RXC1)) {
		if (x_enqueue(&cb_rx1, UDR1)) {
			// Notify Z80 if interrupt setting is enable
			if (z80_int_num_rx1 < 128) {
#if 1
				// CAUTION: vector is NOT interrupt number(0-127)
				Z80_EXTINT_low(z80_int_num_rx1 << 1);
				//_delay_us(1);
				//Z80_EXTINT_High();
#else
				Z80_NMI();		// debug
#endif
			}
		}
	}
}

///////////////////////////////////////////////////////////////////
// Dequeue a character from TX1 buffer and transmit it
// (called from the periodic time ISR)
///////////////////////////////////////////////////////////////////-
void Transmit_TX1_Buf(void)
{
	char data;
	while ((data = x_dequeue(&cb_tx1)) != '\0') {
		USART1_Transmit(data);
	}
}

///////////////////////////////////////////////////////////////////
// Condole device emulation
//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// CAUTION: Followings are PROHIBITED here
//  * XMEM external SRAM R/W
//  * invoke /BUSREQ
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
// Port 01H
//
void OUT_01_CON_SetInterrupt(uint8_t data)
{
	z80_int_num_rx1 = data;
//	x_printf("OUT1:%x\n", data);
}
uint8_t IN_01_CON_GetInterrupt()
{
	return z80_int_num_rx1;
}

//
// Port 02H
//
void OUT_02_CON_Output(uint8_t data)
{
	// set character TX1 console buffer
	x_enqueue(&cb_tx1, data);
//	x_printf("OUT2:%x %d\n", data, cb_tx1.count);
}
uint8_t IN_02_CON_Input()
{
	// Get a character from RX1 console input buffer
	char data = x_dequeue(&cb_rx1);
//	x_printf("IN2:%x %d\n", data, cb_rx1.count);
	return data;
}

//
// Port 03H
//
void OUT_03_CON_Flush(uint8_t data)
{
	// Flush RX1 console input buffer
	x_flush(&cb_rx1);
}
uint8_t IN_03_CON_Status()
{
	// Get remaining characters in RX1 console input buffer
	return cb_rx1.count & 0xff;
}
