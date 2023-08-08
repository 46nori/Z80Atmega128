/*
 * em_debugger.h
 *
 * Created: 2023/08/03 22:50:16
 *  Author: 46nori
 */ 

#ifndef EM_DEBUGGER_H_
#define EM_DEBUGGER_H_
#include "asf.h"

extern void init_em_debugger(void);
extern void OUT_1D_DEBUG_IntLevel(uint8_t data);
extern uint8_t IN_1D_DEBUG_IntLevel(void);
extern void OUT_1E_DEBUG_BreakPointAddress(uint8_t data);
extern uint8_t IN_1E_DEBUG_BreakPointAddress(void);

extern int dbg_DeleteBreakPoint(uint8_t *p);
extern int dbg_AddBreakPoint(uint8_t *p);
extern void dbg_ListBreakPoint(void);
extern void dbg_Continue(void);

#endif /* EM_DEBUGGER_H_ */