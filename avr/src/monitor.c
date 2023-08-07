/*
 * monitor.c
 *
 * Created: 2023/03/31 20:22:13
 *  Author: 46nori
 */ 
#include <asf.h>
#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "monitor.h"
#include "xconsoleio.h"
#include "xmodem.h"
#include "z80io.h"
#include "intelhex.h"
#include "sdcard.h"
#include "emuldev/em_debugger.h"

#define DLIMITER	" "
#define MAX_TOKENS	3
#define T_CMD		0
#define T_PARAM1	1
#define T_PARAM2	2
typedef struct token_list {
	int n;						// number of tokens
	char *token[MAX_TOKENS];	// pointer to token
} token_list;

static token_list *tokenizer(char *str, const char *delim, token_list *t);
static int exec_command(token_list *t);

#define NO_ERROR		(0)
#define ERR_COMMAND		(-1)
#define ERR_PARAM_VAL	(-2)
#define ERR_PARAM_MISS	(-3)

/*********************************************************
 * Monitor main
 *********************************************************/
void monitor(void) {
	token_list tokens;
	char cmd_line[32];

	x_puts("\nATmega128 Tiny Monitor");
	while(1) {
		x_putchar('>');
		x_gets_s(cmd_line, sizeof(cmd_line)-1);
		tokenizer(cmd_line, DLIMITER, &tokens);
		switch (exec_command(&tokens)) {
		case NO_ERROR:
			break;
		case ERR_COMMAND:
			x_puts("not found.");
			break;		
		case ERR_PARAM_VAL:
			x_puts("param error.");
			break;
		case ERR_PARAM_MISS:
			x_puts("missing param.");
			break;
		default:
			x_puts("error!");
			break;
		}
	}
}

// Tokenizer
static token_list *tokenizer(char *str, const char *delim, token_list *t) {
	char *token;
	t->n = 0;
	token = strtok(str, delim);
    while (token != NULL && t->n < MAX_TOKENS) {
		t->token[t->n++] = token;
		token = strtok(NULL, delim);
	}
	return t;
}

// Get token value as an unsigned int
static int get_uint(token_list *t, unsigned int idx, unsigned int *val) {
	if (idx >= t->n) {
		return ERR_PARAM_MISS;
	}

	unsigned int tmp;
	const char *str = t->token[idx];
	if (*str == '$') {
		// Hex
		if (sscanf(str + 1, "%x", &tmp) == 1) {
		    *val = tmp;
		    return NO_ERROR;
		}
	} else if (sscanf(str, "%u", &tmp) == 1) {
		// Decimal
		*val = tmp;
        return NO_ERROR;
    }
    return ERR_PARAM_VAL;
}

//
// User command handler
//
static int c_help(token_list *t);
static int c_dump_flashrom(token_list *t);
static int c_dump_internal_ram(token_list *t);
static int c_dump_external_ram(token_list *t);
static int c_read_internal_ram(token_list *t);
static int c_read_external_ram(token_list *t);
static int c_write_internal_ram(token_list *t);
static int c_write_external_ram(token_list *t);
static int c_load_ihx_external_ram(token_list *t);
static int c_load_bin_external_ram(token_list *t);
static int c_load_bin_internal_ram(token_list *t);
static int c_save_bin_internal_ram(token_list *t);
static int c_mem(token_list *t);
static int c_z80_reset(token_list *t);
static int c_z80_nmi(token_list *t);
static int c_z80_int(token_list *t);
static int c_z80_status(token_list *t);
static int c_z80_break(token_list *t);
static int c_z80_delete(token_list *t);
static int c_z80_info_break(token_list *t);
static int c_z80_continue(token_list *t);
static int c_avr_sei(token_list *t);
static int c_avr_cli(token_list *t);
static int c_test(token_list *t);

static const struct {
	const char *name;
	int (*func)(token_list *);
} cmd_table[] = {
	{"d",     c_dump_external_ram},
	{"di",    c_dump_internal_ram},
	{"df",    c_dump_flashrom},
	{"r",     c_read_external_ram},
	{"ri",    c_read_internal_ram},
	{"w",     c_write_external_ram},
	{"wi",    c_write_internal_ram},
	{"xload", c_load_ihx_external_ram},
	{"bload", c_load_bin_external_ram},
	{"lx",    c_load_bin_internal_ram},
	{"sx",    c_save_bin_internal_ram},
	{"mem",   c_mem},
	{"reset", c_z80_reset},
	{"nmi",   c_z80_nmi},
	{"int",   c_z80_int},
	{"sts",   c_z80_status},
	{"brk",   c_z80_break},
	{"del",   c_z80_delete},
	{"cont",  c_z80_continue},
	{"ib",    c_z80_info_break},
	{"sei",   c_avr_sei},
	{"cli",   c_avr_cli},
	{"test",  c_test},
	{"h",     c_help},
	{"",      NULL}
};

// Lookup command
static int exec_command(token_list *t) {
	if (t->n == 0) {
		return NO_ERROR;	// do nothing
	}
	for (int i = 0; cmd_table[i].func != NULL; i++) {
		if (!strcmp(cmd_table[i].name, t->token[T_CMD])) {
			return cmd_table[i].func(t);
		}
	}
	return ERR_COMMAND;		// command not found
}

/*********************************************************
 * Help
 *********************************************************/
static int c_help(token_list *t) {
static const char help_str[] PROGMEM =	\
	"   <> : mandatory\n"\
	"   [] : optional\n"\
	"   $  : Prefix of hexadecimal\n"\
	"h               : help\n"\
	"== AVR Commands ==\n"\
	"r  [adr]        : read  an External RAM byte\n"\
	"ri [adr]        : read  an Internal RAM byte\n"\
	"w  <adr> <dat>  : write an External RAM byte\n"\
	"wi <adr> <dat>  : write an Internal RAM byte\n"\
	"d  [adr] [len]  : dump External RAM\n"\
	"di [adr] [len]  : dump Internal RAM\n"\
	"df [adr] [len]  : dump FlashROM\n"\
	"lx <adr>        : load binary by XMODEM\n"\
	"sx <adr> <len>  : save binary by XMODEM\n"\
	"mem             : remaining Internal RAM size\n"\
	"sei             : enable  interrupt\n"\
	"cli             : disable interrupt\n"\
	"test [adr]      : XMEM R/W test\n"\
	"== Z80 Commands ==\n"\
	"xload           : load INTEL HEX by XMODEM\n"\
	"bload [adr]     : load binary by XMODEM\n"\
	"reset [0]       : reset, and HALT if 0\n"\
	"nmi             : NMI\n"\
	"int <dat>       : interrupt\n"\
	"sts             : show Z80 status\n"\
	"brk <adr>       : set breakpoint\n"\
	"del <adr>       : delete n\n"\
	"con             : continue from breakpoint\n"\
	"ib              : info breakpoints\n"\
	"";

#if 1	
	char tmp[sizeof(help_str) + 1];
	strncpy_P(tmp, help_str, sizeof(help_str));
	x_puts(tmp);
#else
	x_puts(help_str);
#endif
	return NO_ERROR;
}

//
// Memory map
//
//             |<---len--->|           |<---len--->|
// |--(1)------+-(2)-|-(3)-+--(4)------+-(5)-|
// 0                0x1100                 0x10000
// |<-----shadow---->|<--------real--------->|
// -(6)--|
//

// Block read from external SRAM
//   dst : external SRAM
//   src : internal SRAM
//   len : transfer length
static const unsigned char *read_extram(const unsigned char *dst,
                                        const unsigned char *src,
                                        size_t len) {
	size_t rest;
	unsigned int offset;
	ExtMem_attach();
	if (src < (unsigned char *)INTERNAL_RAM_SIZE) {
		offset = (unsigned int)ExtMem_map();
		if (src <= (unsigned char *)INTERNAL_RAM_SIZE - len) {
			// src is in (1) : shadow
			memcpy((void *)dst, (void *)(offset + src), len);
			ExtMem_unmap();
		} else {
			// src is in (2) : shadow
			rest = (unsigned char *)INTERNAL_RAM_SIZE - (unsigned char *)src;
			memcpy((void *)dst, (void *)(offset + src), rest);
			ExtMem_unmap();
			// src is in (3) : real
			memcpy((void *)(dst+rest), (void *)(src + rest), len - rest);
		}
	} else if (src <= (unsigned char *)EXTERNAL_RAM_SIZE - len) {
		// src is in (4) : real
		memcpy((void *)dst, (void *)src, len);
	} else {
		// src is in (5) : real
		rest = (unsigned char *)EXTERNAL_RAM_SIZE - (unsigned char *)src;
		memcpy((void *)dst, src, rest);
		// src is in (6) : shadow
		offset = (unsigned int)ExtMem_map();
		memcpy((void *)(dst + rest), (void *)offset, len - rest);
		ExtMem_unmap();
	}
	ExtMem_detach();
	return dst;
}

// Block write to external SRAM
//   dst : internal SRAM
//   src : external SRAM
//   len : transfer length
static const unsigned char *write_extram(const unsigned char *dst,
                                         const unsigned char *src,
                                         size_t len) {
	size_t rest;
	unsigned int offset;
	ExtMem_attach();
	if (dst < (unsigned char *)INTERNAL_RAM_SIZE) {
		offset = (unsigned int)ExtMem_map();
		if (src <= (unsigned char *)INTERNAL_RAM_SIZE - len) {
			// dst is in (1) : shadow
			memcpy((void *)(offset + dst), (void *)src, len);
			ExtMem_unmap();
		} else {
			// dst is in (2) : shadow
			rest = (unsigned char *)INTERNAL_RAM_SIZE - (unsigned char *)dst;
			memcpy((void *)(offset + dst), (void *)src, rest);
			ExtMem_unmap();
			// dst is in (3) : real
			memcpy((void *)(dst+rest), (void *)(src + rest), len - rest);
		}
	} else if (dst <= (unsigned char *)EXTERNAL_RAM_SIZE - len) {
		// dst is in (4) : real
		memcpy((void *)dst, (void *)src, len);
	} else {
		// dst is in (5) : real
		rest = (unsigned char *)EXTERNAL_RAM_SIZE - (unsigned char *)dst;
		memcpy((void *)dst, src, rest);
		// dst is in (6) : shadow
		offset = (unsigned int)ExtMem_map();
		memcpy((void *)(dst + rest), (void *)offset, len - rest);
		ExtMem_unmap();
	}
	ExtMem_detach();
	return dst;
}

//
// Hex dump
//
enum memory_type {
	IntROM,		// Internal Flash ROM
	IntRAM,		// Internal RAM
	ExtRAM		// External RAM
};

#define D_COLUMN 16
static int dump(token_list *t, uint8_t **adr, enum memory_type mtype) {
	unsigned int tmp, len = D_COLUMN;
	switch (t->n) {
		case 3:
		if (get_uint(t, T_PARAM2, &len) != NO_ERROR) {
			return ERR_PARAM_VAL;     // parameter error
		}
		case 2:
		if (get_uint(t, T_PARAM1, &tmp) != NO_ERROR) {
			return ERR_PARAM_VAL;     // parameter error
		}
		*adr = (uint8_t *)tmp;
	}

	uint8_t col[D_COLUMN];
	int mod, line = len / D_COLUMN;
	do {
		if (line > 0) {
			mod = D_COLUMN;
		} else if ((mod = len % D_COLUMN) == 0) {
			break;
		}

		x_printf("$%04x ", *adr);

		switch (mtype) {
		case IntROM:
			memcpy_P(col, *adr, D_COLUMN);
			break;
		case IntRAM:
			memcpy(col, *adr, D_COLUMN);
			break;
		case ExtRAM:
			read_extram(col, *adr, D_COLUMN);
			break;
		default:
			break;
		} 

		for (int i = 0; i < mod; i++) {
			x_printf("%02x ", col[i]);
		}
		for (int i = mod; i < D_COLUMN; i++) {
			x_printf("   ");
		}
		x_putchar(' ');

		char c;
		for (int i = 0; i < mod; i++) {
			c = col[i];
			if (c < 0x20 || c > 0x7e) {
				c = '.';
			}
			x_putchar(c);
		}
		*adr += D_COLUMN;
		x_puts("");
	} while (line--);
	return NO_ERROR;
}

/*********************************************************
 * Dump internal SRAM
 *********************************************************/
static int c_dump_internal_ram(token_list *t) {
	static uint8_t *adr = 0;
	return dump(t, &adr, IntRAM);
}

/*********************************************************
 * Dump external SRAM
 *********************************************************/
static int c_dump_external_ram(token_list *t) {
	static uint8_t *adr = 0;
	return dump(t, &adr, ExtRAM);
}

/*********************************************************
 * Dump flash ROM
 *********************************************************/
static int c_dump_flashrom(token_list *t) {
	static uint8_t *adr = 0;
	return dump(t, &adr, IntROM);
}

/*********************************************************
 * Read a byte from internal SRAM
 *********************************************************/
static int c_read_internal_ram(token_list *t) {
	static uint8_t *adr = 0;
	if (t->n > 1) {
		unsigned int tmp;
		if (get_uint(t, T_PARAM1, &tmp) != NO_ERROR) {
			return ERR_PARAM_VAL;
		}
		adr = (uint8_t *)tmp;
	}
	x_printf("%04x: %02x\n", adr, *adr);
	++adr;
	return NO_ERROR;
}

/*********************************************************
 * Read a byte from external SRAM
 *********************************************************/
static int c_read_external_ram(token_list *t) {
    static uint8_t *adr = 0;
    if (t->n > 1) {
	    unsigned int tmp;
	    if (get_uint(t, T_PARAM1, &tmp) != NO_ERROR) {
			return ERR_PARAM_VAL;
		}
		adr = (uint8_t *)tmp;
    }

	volatile uint8_t *a = adr, d;
	ExtMem_attach();
	if (a < (uint8_t *)INTERNAL_RAM_SIZE) {
		a += (unsigned int)ExtMem_map();
		d = *a;
		ExtMem_unmap();
	} else {
		d = *a;		
	}
	ExtMem_detach();
    x_printf("$%04x (%04x): %02x\n", adr, a, d);
	++adr;
	return NO_ERROR;
}

/*********************************************************
 * Write a byte into internal SRAM
 *********************************************************/
static int c_write_internal_ram(token_list *t) {
	if (t->n < 3) {
		return ERR_PARAM_MISS;     // missing parameters
	}

	unsigned int adr, dat;
	if (get_uint(t, T_PARAM1, &adr) != NO_ERROR) {
		return ERR_PARAM_VAL;     // parameter error
	}
	if (get_uint(t, T_PARAM2, &dat) != NO_ERROR) {
		return ERR_PARAM_VAL;     // parameter error
	}
	if (dat > 0xff) {
		return ERR_PARAM_VAL;
	}
	*(volatile uint8_t *)adr = dat;
	return NO_ERROR;
}

/*********************************************************
 * Write a byte into external SRAM
 *********************************************************/
static int write_a_byte_to_external_ram(unsigned int adr, unsigned int dat) {
	ExtMem_attach();
	if (adr < INTERNAL_RAM_SIZE) {
		*(volatile uint8_t *)(adr + (unsigned int)ExtMem_map()) = dat;
		ExtMem_unmap();
	} else {
		*(volatile uint8_t *)adr = dat;
		//x_printf_TX1("%x : %x\n", adr, dat);
	}
	ExtMem_detach();
	return 0;
}

static int c_write_external_ram(token_list *t) {
	if (t->n < 3) {
		return ERR_PARAM_MISS;     // missing parameters
	}

	unsigned int adr, dat;
	if (get_uint(t, T_PARAM1, &adr) != NO_ERROR) {
		return ERR_PARAM_VAL;     // parameter error
	}
	if (get_uint(t, T_PARAM2, &dat) != NO_ERROR) {
		return ERR_PARAM_VAL;     // parameter error
	}
	if (dat > 0xff) {
		return ERR_PARAM_VAL;
	}

	write_a_byte_to_external_ram(adr, dat);

	return NO_ERROR;
}

//
// Load by XMODEM
//
static int load_xmodem(unsigned int dest, copyfunc cfunc) {
	size_t size;
	x_puts("Start XMODEM within 90s...");
	switch (r_xmodem((unsigned char *)dest, &size, cfunc)) {
	case -1:
		x_puts("\nTransfer error.");
		break;
	case -2:
		x_puts("\nTimed out.");
		break;
	case -4:
		x_puts("\nCopy error.");
		break;
	default:
		x_printf("\nReceived %d bytes.\n", size);
		break;
	}
	return NO_ERROR;
}

/*********************************************************
 * Download INTEL HEX format into External SRAM
 *********************************************************/
static struct ix_ctx ctx;
// convert IHX in a XMODEM packet to binary and write to external memory  
static int copy_ihx_xmem(unsigned char *dst, unsigned char *src, size_t size) {
	while (size-- > 0) {
		if (push_ix(&ctx, *src++) >= 2) {
			return -4;	// copy error
		}
	}
	return 0;
}

static int c_load_ihx_external_ram(token_list *t) {
	init_ix(&ctx, write_a_byte_to_external_ram);
	load_xmodem(0, copy_ihx_xmem);
	switch (ctx.status) {
	case 2:
		x_puts("IHX check sum error.");
		break;
	case 3:
		x_puts("IHX sequence error.");
		break;
	case 4:
		x_puts("bug!");
		break;
	default:
		break;
	}
	return NO_ERROR;
}

/*********************************************************
 * Download binary into external SRAM
 *********************************************************/
// copy XMODEM packet to external memory
static int copy_bin_xmem(unsigned char *dst, unsigned char *src, size_t size) {
	write_extram(dst, src, size);
	return 0;
}

static int c_load_bin_external_ram(token_list *t) {
    if (t->n < 2) {
	    return ERR_PARAM_MISS;	// missing parameters
    }
    unsigned int dest;
    if (get_uint(t, T_PARAM1, &dest) != NO_ERROR) {
	    return ERR_PARAM_VAL;	// parameter error
    }
	load_xmodem(dest, copy_bin_xmem);
	return NO_ERROR;
}

/*********************************************************
 * Download binary into internal SRAM
 *********************************************************/
static int c_load_bin_internal_ram(token_list *t) {
    if (t->n < 2) {
	    return ERR_PARAM_MISS;	// missing parameters
    }
    unsigned int dest;
    if (get_uint(t, T_PARAM1, &dest) != NO_ERROR) {
	    return ERR_PARAM_VAL;	// parameter error
    }
	return load_xmodem(dest, NULL);
}

/*********************************************************
 * Save by XMODEM
 *********************************************************/
static int c_save_bin_internal_ram(token_list *t) {
    if (t->n < 3) {
        return ERR_PARAM_MISS;	// missing parameters
    }
    unsigned int src, size;
    if (get_uint(t, T_PARAM1, &src) != NO_ERROR) {
        return ERR_PARAM_VAL;	// parameter error
    }
    if (get_uint(t, T_PARAM2, &size) != NO_ERROR) {
        return ERR_PARAM_VAL;	// parameter error
    }

    size_t blks = size / 128;
    if (size % 128) {
        ++blks;
    }
	switch (s_xmodem((unsigned char *)src, blks)) {
		case -1:
			x_puts("\nTransfer error.");
			break;
		case -2:
			x_puts("\nTimed out.");
			break;
		case -3:
			x_puts("\nAborted by receiver.");
			break;
		default:
			x_printf("\nSent %d packets.\n", blks);
			break;
	}
	return NO_ERROR;
}

/*********************************************************
 * Calculate remaining size of internal RAM
 *********************************************************/
static int c_mem(token_list *t) {
	extern int __heap_start, *__brkval;
	int v;
	x_printf("%d bytes left.\n",
	(int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval));
	return NO_ERROR;
}

/*********************************************************
 * Invoke reset of Z80
 *********************************************************/
static int c_z80_reset(token_list *t) {
    if (t->n > 1) {
		// HALT after reset if 0 is presented as parameter
	    unsigned int tmp;
	    if (get_uint(t, T_PARAM1, &tmp) != NO_ERROR) {
		    return ERR_PARAM_VAL;
	    }
	    if (tmp == 0) {
			Z80_HALT();
		}
    }
	Z80_RESET();
	return NO_ERROR;
}

/*********************************************************
 * Invoke NMI of Z80
 *********************************************************/
static int c_z80_nmi(token_list *t) {
	Z80_NMI();
	return NO_ERROR;
}

/*********************************************************
 * Invoke INT of Z80
 *********************************************************/
static int c_z80_int(token_list *t) {
	if (t->n < 2) {
		return ERR_PARAM_MISS;     // missing parameters
	}

	unsigned int dat;
	if (get_uint(t, T_PARAM1, &dat) != NO_ERROR) {
		return ERR_PARAM_VAL;     // parameter error
	}
	Z80_EXTINT(dat);
	return NO_ERROR;
}

/*********************************************************
 * Show Z80 status
 *********************************************************/
static int c_z80_status(token_list *t) {
	x_printf("/BUSRQ:%c\n", Is_Z80_BUSRQ() ? 'H' : 'L');
	x_printf("/HALT :%c\n", Is_Z80_HALT() ?  'H' : 'L');
	return NO_ERROR;
}

/*********************************************************
 * Add breakpoint
 *********************************************************/
static int c_z80_break(token_list *t)
{
	if (t->n < 2) {
		return ERR_PARAM_MISS;     // missing parameters
	}

	unsigned int adr;
	if (get_uint(t, T_PARAM1, &adr) != NO_ERROR) {
		return ERR_PARAM_VAL;     // parameter error
	}
	dbg_AddBreakPoint((uint8_t *)adr);
	return NO_ERROR;
}

/*********************************************************
 * Delete breakpoint
 *********************************************************/
static int c_z80_delete(token_list *t)
{
	if (t->n < 2) {
		return ERR_PARAM_MISS;     // missing parameters
	}

	unsigned int adr;
	if (get_uint(t, T_PARAM1, &adr) != NO_ERROR) {
		return ERR_PARAM_VAL;     // parameter error
	}
	dbg_DeleteBreakPoint((uint8_t *)adr);
	return NO_ERROR;
}

/*********************************************************
 * List breakpoints
 *********************************************************/
static int c_z80_info_break(token_list *t)
{
	dbg_ListBreakPoint();
	return NO_ERROR;
}

/*********************************************************
 * Continue from breakpoint
 *********************************************************/
static int c_z80_continue(token_list *t)
{
	dbg_Continue();
	return NO_ERROR;
}

/*********************************************************
 * Enable AVR interrupt
 *********************************************************/
static int c_avr_sei(token_list *t) {
	sei();
	return NO_ERROR;
}

/*********************************************************
 * Disable AVR interrupt
 *********************************************************/
static int c_avr_cli(token_list *t) {
	cli();
	return NO_ERROR;
}

/*********************************************************
 * Test external memory
 *********************************************************/
static int c_test(token_list *t) {
	// External memory test
    unsigned int offset = 0;
    if (t->n > 1) {
	    unsigned int tmp;
	    if (get_uint(t, T_PARAM1, &tmp) != NO_ERROR) {
		    return ERR_PARAM_VAL;
	    }
	    offset = tmp;
    }

	ExtMem_attach();
	
	uint8_t *adr = (uint8_t *)offset;
	bool isShadow = false;
	if (offset < INTERNAL_RAM_SIZE - 0x600) {
		adr = (uint8_t *)ExtMem_map() + offset;
		isShadow = true;
	}

	int i;
	// Write phase
	unsigned int w_sum = 0;
	for (i = 0; i < 0x100; adr[i] = 0x55, w_sum += 0x55, i++);
	for (     ; i < 0x200; adr[i] = i,    w_sum += i,    i++);
	for (     ; i < 0x300; adr[i] = 0xaa, w_sum += 0xaa, i++);
	for (     ; i < 0x400; adr[i] = 0x00, w_sum += 0x00, i++);
	for (     ; i < 0x500; adr[i] = 0xff, w_sum += 0xff, i++);
	// Read phase
	unsigned int r_sum = 0;
	for (     ; i > 0; r_sum += adr[--i]);

	if (isShadow) {
		ExtMem_unmap();
	}
	ExtMem_detach();

	x_printf("write sum=%x\n", w_sum);
	x_printf("read  sum=%x\n", r_sum);
	if (w_sum != r_sum) {
		dump(t, &adr, ExtRAM);
		x_puts("XMEM NG!\n");
	} else {
		x_puts("XMEM OK!\n");
	}
	
	// SD Card Test	
	if (sdcard_test() == 0) {
		x_puts("SD Card OK.");
		} else {
		x_puts("SD Card NG.");
	}
	
	return NO_ERROR;
}