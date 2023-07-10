/*
 * em_led.c
 *
 * Created: 2023/07/08 16:36:24
 *  Author: 46nori
 */ 

#include "asf.h"
#include "em_led.h"

void init_em_led(void)
{
}

void OUT_1F_LED_Set(uint8_t data)
{
	PORTE = (PORTE & 0x1f) | ((data & 0x07) << 5);
}

uint8_t IN_1F_LED_Get(void)
{
	return PORTE >> 5;
}
