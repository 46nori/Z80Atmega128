/**
 * \file
 *
 * \brief User board definition template
 *
 */

 /* This file is intended to contain definitions and configuration details for
 * features and devices that are available on the board, e.g., frequency and
 * startup time for an external crystal, external memory devices, LED and USART
 * pins.
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#ifndef USER_BOARD_H
#define USER_BOARD_H

#include <conf_board.h>
#define F_CPU 16000000UL
#include <util/delay.h>

// GPIO
#define SET_BIT(port, bit)   ((port) |=  _BV(bit))
#define CLR_BIT(port, bit)   ((port) &= ~_BV(bit))
#define SET_BYTE(port, byte) ((port) = (uint8_t)(byte))
#define GET_BYTE(port)       (port)

#endif // USER_BOARD_H
