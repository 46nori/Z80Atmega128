/*
 * em_led.h
 *
 * Created: 2023/07/08 16:36:02
 *  Author: 46nori
 */ 

#ifndef EM_LED_H_
#define EM_LED_H_

extern void init_em_led(void);
extern void OUT_1F_LED_Set(uint8_t data);
extern uint8_t IN_1F_LED_Get(void);
extern void em_led_heartbeat(int f);

#endif /* EM_LED_H_ */