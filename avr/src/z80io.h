/*
 * z80io.h
 *
 * Created: 2023/04/07 21:43:41
 *  Author: 46nori
 */ 

#ifndef Z80IO_H_
#define Z80IO_H_
#include <asf.h>

#define INTERNAL_RAM_SIZE	0x1100
#define EXTERNAL_RAM_SIZE	0x10000

extern void ExtMem_Init(void);
extern void ExtMem_attach(void);
extern void ExtMem_detach(void);
extern void *ExtMem_map(void);
extern void ExtMem_unmap(void);

extern void Z80_BUSREQ(int st);
extern void Z80_RESET(void);
extern void Z80_NMI(void);
extern void Z80_EXTINT(uint8_t vector);
extern void Z80_EXTINT_low(uint8_t vector);
extern void Z80_EXTINT_High(void);
extern void Z80_CLRWAIT(void);
extern void Z80_HALT(void);
extern int Is_Z80_BUSRQ(void);
extern int Is_Z80_BUSACK(void);
extern int Is_Z80_HALT(void);

extern volatile uint8_t z80_int_vector;

#endif /* Z80IO_H_ */