/*
 * z80io.h
 *
 * Created: 2023/04/07 21:43:41
 *  Author: 46nori
 */ 

#ifndef Z80IO_H_
#define Z80IO_H_

enum shadow_size {
	MAP_32K = 0b00000111,
	MAP_16K = 0b00000110,
	MAP_8K  = 0b00000101,
	MAP_4K  = 0b00000100,
	MAP_2K  = 0b00000011,
	MAP_1K  = 0b00000010,
	MAP_256 = 0b00000001,
	UNMAP   = 0b00000000
};

extern void ExtMemory_init(void);
extern void ExtMemory_attach(void);
extern void ExtMemory_detach(void);
extern void *ExtMemory_map(enum shadow_size size);

extern void Z80_BUSREQ(int st);
extern void Z80_RESET(void);
extern void Z80_INT_REQ(void);
void Z80_CLRWAIT(void);

#endif /* Z80IO_H_ */