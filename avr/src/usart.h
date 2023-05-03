/*
 * usart.h
 *
 * Created: 2023/03/31 17:39:44
 *  Author: 46nori
 */ 


#ifndef USART_H_
#define USART_H_
#include <asf.h>

#define FOSC F_CPU

#ifndef EOF
#define EOF (-1)
#endif

extern void USART0_Init(uint32_t baud);
extern void USART1_Init(uint32_t baud);
extern void USART0_Transmit(uint8_t data);
extern void USART1_Transmit(uint8_t data);
extern uint8_t USART0_Receive(void);
extern uint8_t USART1_Receive(void);
extern int USART0_Receive_tout(uint32_t us);

#endif /* USART_H_ */