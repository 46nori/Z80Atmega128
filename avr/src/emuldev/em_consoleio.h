/*
 * em_consoleio.h
 *
 * Created: 2023/06/28 22:29:33
 *  Author: 46nori
 */ 


#ifndef EM_CONSOLEIO_H_
#define EM_CONSOLEIO_H_

#include <asf.h>

// Console I/O
extern void init_em_console(void);

extern void OUT_01_CON_SetInterrupt(uint8_t data);
extern void OUT_02_CON_Output(uint8_t data);
extern void OUT_03_CON_Flush(uint8_t data);
extern uint8_t IN_01_CON_GetInterrupt(void);
extern uint8_t IN_02_CON_Input(void);
extern uint8_t IN_03_CON_Status(void);

extern void EnqueueRX1_NotifyZ80(void);
extern void Transmit_TX1_Buf(void);

#endif /* EM_CONSOLEIO_H_ */