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

static uint8_t *bp_adr = 0;
static uint8_t bp_adr_state = 0;
uint8_t IN_1E_DEBUG_BreakPointAddress(void)
{
	bp_adr_state = 0;
	return 0;
}

void OUT_1E_DEBUG_BreakPointAddress(uint8_t data)
{
	if (bp_adr_state == 0) {
		// Get higher address of breakpoint
		bp_adr = (uint8_t *)((unsigned int)data << 8);
		bp_adr_state = 1;
	} else {
		// Get lower address of breakpoint
		bp_adr = (uint8_t *)((unsigned int)bp_adr | data);
		bp_adr_state = 0;
		
		x_printf(">>>Break! at $%04x", bp_adr);
		if (FindBreakPoint(bp_adr) == -1) {
			x_puts(" (unexpected)");			
		} else {
			x_puts("");
		}
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