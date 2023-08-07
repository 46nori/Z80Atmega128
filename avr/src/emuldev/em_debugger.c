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

struct BreakPoint {
	uint8_t *address;
	uint8_t opcode;
	bool in_use;
};
#define NUM_OF_BP	10
static struct BreakPoint bp_table[NUM_OF_BP];

void init_em_debugger(void)
{
	for (int i = 0; i < NUM_OF_BP; i++) {
		bp_table[i].in_use = false;
	}
}

static uint8_t debugger_int_level = 128;
void OUT_1D_DEBUG_IntLevel(uint8_t data)
{
	debugger_int_level = data;
}

uint8_t IN_1D_DEBUG_IntLevel(void)
{
	return debugger_int_level;
}

void OUT_1E_DEBUG_OUT(uint8_t data)
{
	x_printf(">>>Break! (%x)\n", data);
}

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
		// CAUTION: vector is NOT interrupt number(0-127)
		Z80_EXTINT_low(debugger_int_level << 1);
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