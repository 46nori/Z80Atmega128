/*
 * z80io.h
 *
 * Created: 2023/04/07 21:43:41
 *  Author: 46nori
 */ 

#ifndef Z80IO_H_
#define Z80IO_H_
#include <asf.h>

extern void ExtMem_Init(void);
extern void ExtMem_attach(void);
extern void ExtMem_detach(void);
extern void *ExtMem_map(void);
extern void ExtMem_unmap(void);

extern void Z80_BUSREQ(int st);
extern void Z80_RESET(void);
extern void Z80_NMI(void);
extern void Z80_EXTINT(uint8_t vector);
extern void Z80_CLRWAIT(void);
extern void Z80_HALT(void);

extern uint8_t z80_intvect;

#endif /* Z80IO_H_ */