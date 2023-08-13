/*
 * em_debugger.c
 *
 * Created: 2023/08/03 22:50:43
 *  Author: 46nori
 */ 
#include <string.h>
#include "asf.h"
#include "em_debugger.h"
#include "z80io.h"
#include "xconsoleio.h"

struct Z80Registers {
	uint8_t I;
	uint8_t A;
	uint8_t F;
	uint8_t B;
	uint8_t C;
	uint8_t D;
	uint8_t E;
	uint8_t H;
	uint8_t L;
	uint8_t A_;
	uint8_t F_;
	uint8_t B_;
	uint8_t C_;
	uint8_t D_;
	uint8_t E_;
	uint8_t H_;
	uint8_t L_;
	uint16_t IX;
	uint16_t IY;
	uint16_t SP_;
	uint16_t STACK[3];
};

#define NUM_OF_BP	10
struct BreakPoint {
	uint8_t *address;
	uint8_t opcode;
	bool in_use;
};
static struct BreakPoint bp_table[NUM_OF_BP];

void init_em_debugger(void)
{
	for (int i = 0; i < NUM_OF_BP; i++) {
		bp_table[i].in_use = false;
		bp_table[i].address = (uint8_t *)0xffff;
	}
}

//
//  Utilities for breakpoint
//
static void SetBreakPoint(struct BreakPoint *bp,  uint8_t *p)
{
	unsigned int offset = 0;

	bp->in_use = true;
	bp->address = p;
	ExtMem_attach();
	if (p < (uint8_t *)INTERNAL_RAM_SIZE) {
		offset = (unsigned int)ExtMem_map();
	}
	bp->opcode = *(p + offset);
	*(p + offset) = 0xff;			// Set 'RST 7'
	if (p < (uint8_t *)INTERNAL_RAM_SIZE) {
		ExtMem_unmap();
	}
	ExtMem_detach();
}

static void ClearBreakPoint(struct BreakPoint *bp, uint8_t *p)
{
	ExtMem_attach();
	if (p < (uint8_t *)INTERNAL_RAM_SIZE) {
		*(bp->address + (unsigned int)ExtMem_map()) = bp->opcode;
		ExtMem_unmap();
		} else {
		*bp->address = bp->opcode;
	}
	ExtMem_detach();
	bp->in_use = false;
	bp->address = (uint8_t *)0xffff;
}

static int FindBreakPoint(uint8_t *p)
{
	for (int i = 0; i < NUM_OF_BP - 1; i++) {
		struct BreakPoint *bp = &bp_table[i];
		if (bp->in_use == true && bp->address == p) {
			return i;
		}
	}
	return -1;
}

static void swap_bp(struct BreakPoint *a, struct BreakPoint *b) {
	struct BreakPoint tmp;
	memcpy(&tmp, a, sizeof(struct BreakPoint));
	memcpy(a, b, sizeof(struct BreakPoint));
	memcpy(b, &tmp, sizeof(struct BreakPoint));
}

static void SortBreakPoint(void)
{
	for (int i = 0; i < NUM_OF_BP - 1; i++) {
		for (int j = 0; j < NUM_OF_BP - i - 1; j++) {
			if (bp_table[j].address > bp_table[j + 1].address) {
				swap_bp(&bp_table[j], &bp_table[j + 1]);
			}
		}
	}
}


//
//   handler (called from isr.c)
//
static uint8_t debugger_int_level = 128;
void OUT_1D_DEBUG_IntLevel(uint8_t data)
{
	debugger_int_level = data;
}

uint8_t IN_1D_DEBUG_IntLevel(void)
{
	return debugger_int_level;
}

static uint8_t *bp_adr;
static int dbginfo_state = 0;
uint8_t IN_1E_DEBUG_BreakPointAddress(void)
{
	dbginfo_state = 0;
	return 0;
}

void OUT_1E_DEBUG_BreakPointAddress(uint8_t data)
{
	static struct Z80Registers z80reg = {0};
	switch (dbginfo_state) {
	case 0:		// Breakpoint high
		bp_adr = (uint8_t *)((uint16_t)data << 8);
		dbginfo_state = 1;
		break;
	case 1:		// Breakpoint low
		bp_adr += data;
		dbginfo_state = 2;
		break;
	case 2:		// SP high
		z80reg.SP_ = (uint16_t)data << 8;
		dbginfo_state = 3;
		break;
	case 3:		// SP low
		z80reg.SP_ |= data;
		dbginfo_state = 4;
		break;
	case 4:		// I
		z80reg.I = data;
		dbginfo_state = 5;
		break;
	case 5:		// IY high
		z80reg.IY = (uint16_t)data << 8;
		dbginfo_state = 6;
		break;
	case 6:		// IY low
		z80reg.IY |= data;
		dbginfo_state = 7;
		break;
	case 7:		// IX high
		z80reg.IX = (uint16_t)data << 8;
		dbginfo_state = 8;
		break;
	case 8:		// IX low
		z80reg.IX |= data;
		dbginfo_state = 9;
		break;
	case 9:		// H
		z80reg.H = data;
		dbginfo_state = 10;
		break;
	case 10:	// L
		z80reg.L = data;
		dbginfo_state = 11;
		break;
	case 11:	// D
		z80reg.D = data;
		dbginfo_state = 12;
		break;
	case 12:	// E
		z80reg.E = data;
		dbginfo_state = 13;
		break;
	case 13:	// B
		z80reg.B = data;
		dbginfo_state = 14;
		break;
	case 14:	// C
		z80reg.C = data;
		dbginfo_state = 15;
		break;
	case 15:	// A
		z80reg.A = data;
		dbginfo_state = 16;
		break;
	case 16:	// F
		z80reg.F = data;
		dbginfo_state = 17;
		break;
	case 17:	// A'
		z80reg.A_ = data;
		dbginfo_state = 18;
		break;
	case 18:	// F'
		z80reg.F_ = data;
		dbginfo_state = 19;
		break;
	case 19:	// H'
		z80reg.H_ = data;
		dbginfo_state = 20;
		break;
	case 20:	// L'
		z80reg.L_ = data;
		dbginfo_state = 21;
		break;
	case 21:	// D'
		z80reg.D_ = data;
		dbginfo_state = 22;
		break;
	case 22:	// E'
		z80reg.E_ = data;
		dbginfo_state = 23;
		break;
	case 23:	// B'
		z80reg.B_ = data;
		dbginfo_state = 24;
		break;
	case 24:	// C'
		z80reg.C_ = data;
		dbginfo_state = 25;
		break;
	case 25:
		z80reg.STACK[0] = (uint16_t)data << 8;
		dbginfo_state = 26;
		break;
	case 26:
		z80reg.STACK[0] |= data;
		dbginfo_state = 27;
		break;
	case 27:
		z80reg.STACK[1] = (uint16_t)data << 8;
		dbginfo_state = 28;
		break;
	case 28:
		z80reg.STACK[1] |= data;
		dbginfo_state = 29;
		break;
	case 29:
		z80reg.STACK[2] = (uint16_t)data << 8;
		dbginfo_state = 30;
		break;
	case 30:
		z80reg.STACK[2] |= data;
	default :
		dbginfo_state = 0;
		x_printf(">>>Break! at $%04x", bp_adr);
		int idx = FindBreakPoint(bp_adr);
		if (idx == -1) {
			x_puts(" (unexpected)");
		} else {
			x_puts("");
		}
		x_printf("AF=$%02x%02x [Z:%d C:%d] I=$%02x\n",	z80reg.A, z80reg.F,
														z80reg.F & 0x40 ? 1:0,
														z80reg.F & 0x01 ? 1:0,
														z80reg.I);
		x_printf("BC=$%02x%02x DE=$%02x%02x HL=$%02x%02x\n",
														z80reg.B, z80reg.C, z80reg.D,
														z80reg.E, z80reg.H, z80reg.L);
		x_printf("IX=$%04x IY=$%04x SP=$%04x\n", z80reg.IX, z80reg.IY, z80reg.SP_);
		x_printf("(SP+0)=$%04x\n", z80reg.STACK[2]);
		x_printf("(SP+2)=$%04x\n", z80reg.STACK[1]);
		x_printf("(SP+4)=$%04x\n", z80reg.STACK[0]);
		break;
	}
}

//
//  Debugger command (called from TinyMonitor)
//
int dbg_AddBreakPoint(uint8_t *p)
{
	int i;
	for (i = 0; i < NUM_OF_BP; i++)	{
		struct BreakPoint *bp = &bp_table[i];
		if (bp->in_use == true) {
			if (bp->address == p) {
				break;			// already set
			} else {
				continue;		// in use
			}
		}
		SetBreakPoint(bp, p);
		break;
	}
	if (i == NUM_OF_BP) {
		return -1;				// Full
	}
	SortBreakPoint();
	return 0;					// Success
}

int dbg_DeleteBreakPoint(uint8_t *p)
{
	int i;
	for (i = 0; i < NUM_OF_BP; i++)	{
		struct BreakPoint *bp = &bp_table[i];
		if (bp->in_use == true && bp->address == p) {
			ClearBreakPoint(bp, p);
			break;
		}
	}
	if (i == NUM_OF_BP) {
		return -1;				// Not found
	}
	SortBreakPoint();
	return 0;					// Success	
}

void dbg_Continue(void)
{
	if (debugger_int_level < 128) {
		uint8_t *p = bp_adr;
		dbg_DeleteBreakPoint(p);
		// CAUTION: vector is NOT interrupt number(0-127)
		Z80_EXTINT_low(debugger_int_level << 1);
		_delay_us(10);
		dbg_AddBreakPoint(p);
	} else {
		x_puts("Cannot continue due to no int setting.");
	}
}

void dbg_ListBreakPoint(void)
{
	for (int i = 0; i < NUM_OF_BP; i++)	{
		struct BreakPoint *bp = &bp_table[i];
		if (bp->in_use) {
			x_printf("%d: $%04x $%02x\n", i, bp->address, bp->opcode);
		}		
	}	
}