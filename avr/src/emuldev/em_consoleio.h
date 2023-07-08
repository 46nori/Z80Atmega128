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

extern uint8_t IN_00_CONIN(void);
extern uint8_t IN_01_CONIN_GetStatus(void);
extern uint8_t IN_02_CONIN_GetBufferSize(void);
extern uint8_t IN_03_CONIN_GetIntLevel(void);
extern uint8_t IN_06_CONOUT_GetStatus(void);
extern uint8_t IN_07_CONOUT_GetBufferSize(void);
extern uint8_t IN_08_CONOUT_GetIntLevel(void);
extern void OUT_02_CONIN_Flush(uint8_t data);
extern void OUT_03_CONIN_SetIntLevel(uint8_t data);
extern void OUT_05_CONOUT(uint8_t data);
extern void OUT_07_CONOUT_Flush(uint8_t data);
extern void OUT_08_CONOUT_SetIntLevel(uint8_t data);

extern void EnqueueRX1_NotifyZ80(void);
extern void Transmit_TX1_Buf(void);

#endif /* EM_CONSOLEIO_H_ */

