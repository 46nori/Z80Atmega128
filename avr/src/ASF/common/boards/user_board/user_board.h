/*
 * user_boart.h
 *
 * Created: 2023/03/05 11:29:23
 *  Author: 46nori
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
