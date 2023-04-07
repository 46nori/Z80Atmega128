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
static void exec_command(token_list *t);

/**
 * Monitor main
 */
void monitor(void) {
	token_list tokens;
	char cmd_line[32];

	x_puts("\nATmega128 Tiny Monitor");
	while(1) {
		x_putchar('>');
		x_gets_s(cmd_line, sizeof(cmd_line)-1);
		//x_puts(cmd_line);
		tokenizer(cmd_line, DLIMITER, &tokens);
		exec_command(&tokens);
	}
}

/* Tokenizer */
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

/* Get token value as an unsigned int */
static int get_uint(token_list *t, unsigned int idx, unsigned int *val) {
	if (idx >= t->n) {
		return -1;
	}

	unsigned int tmp;
	const char *str = t->token[idx];
	if (*str == '$') {
		// Hex
		if (sscanf(str + 1, "%x", &tmp) == 1) {
		    *val = tmp;
		    return 0;
		}
	} else if (sscanf(str, "%u", &tmp) == 1) {
		// Decimal
		*val = tmp;
        return 0;
    }
    return -2;
}

//
// User command handler
//
static void c_help(token_list *t);
static void c_dump_flashrom(token_list *t);
static void c_dump_internal_ram(token_list *t);
static void c_dump_external_ram(token_list *t);
static void c_read_byte(token_list *t);
static void c_write_byte(token_list *t);
static void c_load(token_list *t);
static void c_save(token_list *t);
static void c_heap(token_list *t);
static void c_z80_reset(token_list *t);

static const struct {
	const char *name;
	void (*func)(token_list *);
} cmd_table[] = {
	{"d",     c_dump_external_ram},
	{"di",    c_dump_internal_ram},
	{"df",    c_dump_flashrom},
	{"r",     c_read_byte},
	{"w",     c_write_byte},
	{"ld",    c_load},
	{"sv",    c_save},
	{"heap",  c_heap},
	{"h",     c_help},
	{"reset", c_z80_reset},
	{"",      NULL}
};

/* Lookup command */
static void exec_command(token_list *t) {
	for (int i = 0; cmd_table[i].func != NULL; i++) {
		if (!strcmp(cmd_table[i].name, t->token[T_CMD])) {
			cmd_table[i].func(t);
			break;
		}
	}
}

//
// Help
//
static void c_help(token_list *t) {
static const char help_str[] PROGMEM =	\
	"   <> : mandatory\n"\
	"   [] : optional\n"\
	"   $  : Prefix of hexadecimal\n"\
	"h               : help\n"\
	"d  [adr] [len]  : dump external RAM\n"\
	"di [adr] [len]  : dump internal RAM\n"\
	"df [adr] [len]  : dump FlashROM\n"\
	"r  [adr]        : read  a RAM byte\n"\
	"w  <adr> <dat>  : write a RAM byte\n"\
	"ld <adr>        : load by XMODEM\n"\
	"sv <adr> <len>  : save by XMODEM\n"\
	"heap            : remaining heap size\n"\
	"reset           : restart Z80\n"\
	"";

#if 1	
	char tmp[sizeof(help_str) + 1];
	strncpy_P(tmp, help_str, sizeof(help_str));
	x_puts(tmp);
#else
	x_puts(help_str);
#endif
}

//
// Hex dump
//
enum memory_type {
	IntFROM,	// Internal Flash ROM
	IntSRAM,	// Internal SRAM
	ExtSRAM		// External SRAM
};
static const unsigned char *memcpy_extram(const unsigned char *dst,
const unsigned char *src, size_t len);

#define D_COLUMN 16
static void dump(token_list *t, uint8_t **adr, enum memory_type mtype) {
	unsigned int tmp, len = D_COLUMN;
	switch (t->n) {
		case 3:
		if (get_uint(t, T_PARAM2, &len) != 0) {
			return;     // parameter error
		}
		case 2:
		if (get_uint(t, T_PARAM1, &tmp) != 0) {
			return;     // parameter error
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
		case IntFROM:
			memcpy_P(col, *adr, D_COLUMN);
			break;
		case IntSRAM:
			memcpy(col, *adr, D_COLUMN);
			break;
		case ExtSRAM:
			memcpy_extram(col, *adr, D_COLUMN);
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
#define INTERNAL_RAM_SIZE	0x1100
#define EXTERNAL_RAM_SIZE	0x10000
static const unsigned char *memcpy_extram(const unsigned char *dst,
                                          const unsigned char *src, size_t len) {
	size_t rest;
	unsigned int offset;
	if (src < (unsigned char *)INTERNAL_RAM_SIZE) {
		ExtMemory_attach();
		offset = (unsigned int)ExtMemory_map(MAP_8K);
		if (src <= (unsigned char *)INTERNAL_RAM_SIZE - len) {
			// src is in (1) : shadow
			memcpy((void *)dst, (void *)(offset + src), len);
			ExtMemory_map(UNMAP);
			ExtMemory_detach();
		} else {
			// src is in (2) : shadow
			rest = (unsigned char *)INTERNAL_RAM_SIZE - (unsigned char *)src;
			memcpy((void *)dst, (void *)(offset + src), rest);
			ExtMemory_map(UNMAP);
			ExtMemory_detach();
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
		ExtMemory_attach();
		offset = (unsigned int)ExtMemory_map(MAP_8K);
		memcpy((void *)(dst + rest), (void *)offset, len - rest);
		ExtMemory_map(UNMAP);
		ExtMemory_detach();
	}
	return dst;
}

static void c_dump_flashrom(token_list *t) {
	static uint8_t *adr = 0;
	dump(t, &adr, IntFROM);
}

static void c_dump_internal_ram(token_list *t) {
	static uint8_t *adr = 0;
	dump(t, &adr, IntSRAM);
}

static void c_dump_external_ram(token_list *t) {
	static uint8_t *adr = 0;
	dump(t, &adr, ExtSRAM);
}

//
// Read a byte
//
static void c_read_byte(token_list *t) {
    static uint8_t *adr = 0;
    if (t->n > 1) {
        unsigned int tmp;
        if (get_uint(t, T_PARAM1, &tmp) != 0) {
            adr = (uint8_t *)tmp;
        }
    }
    x_printf("%x\n", *adr++);
}

//
// Write a byte
//
static void c_write_byte(token_list *t) {
	if (t->n < 3) {
		return;     // missing parameters
	}

	unsigned int adr, dat;
	if (get_uint(t, T_PARAM1, &adr) != 0) {
		return;     // parameter error
	}
	if (get_uint(t, T_PARAM2, &dat) != 0) {
		return;     // parameter error
	}
	if (dat > 0xff) {
		return;
	}
	*(uint8_t *)adr = dat;
}

//
// Load binary by XMODEM
//
static void c_load(token_list *t) {
    if (t->n < 2) {
        return;     // missing parameters
    }
    unsigned int dest;
    if (get_uint(t, T_PARAM1, &dest) != 0) {
        return;     // parameter error
    }

	size_t size;
	x_puts("Start XMODEM within 90s...");
	switch (r_xmodem((unsigned char *)dest, &size)) {
		case -1:
		x_puts("\nTransfer error.");
		break;
		case -2:
		x_puts("\nTimed out.");
		break;
#if 0
		case -4:
		x_puts("\nCopy error.");
		break;
#endif
		default:
		x_printf("\nReceived %d bytes.\n", size);
		break;
	}
}

//
// Save binary by XMODEM
//
static void c_save(token_list *t) {
    if (t->n < 3) {
        return;     // missing parameters
    }
    unsigned int src, size;
    if (get_uint(t, T_PARAM1, &src) != 0) {
        return;     // parameter error
    }
    if (get_uint(t, T_PARAM2, &size) != 0) {
        return;     // parameter error
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
}

//
// Calc remaining size of heap memory
//
static void c_heap(token_list *t) {
	extern int __heap_start, *__brkval;
	int v;
	x_printf("%d bytes left.\n",
	(int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval));
}

static void c_z80_reset(token_list *t) {
	//(void)t;
	Z80_RESET();
}
