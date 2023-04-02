/*
 * monitor.c
 *
 * Created: 2023/03/31 20:22:13
 *  Author: 46nori
 */ 
#include <asf.h>
#include <stdio.h>
#include <string.h>
#include "monitor.h"
#include "xconsoleio.h"
#include "xmodem.h"
//
// Tokenizer
//
#define MAX_TOKENS 3
#define T_CMD      0
#define T_PARAM1   1
#define T_PARAM2   2
typedef struct token_list {
	int n;						// number of tokens
	char *token[MAX_TOKENS];	// pointer to token
} token_list;

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

//
// Get token value as an unsigned int
//
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
// Command function list
//
static void c_help(token_list *t) {
	x_puts("   <> : mandatory");
	x_puts("   [] : optional");
	x_puts("   $  : Prefix of hexadecimal");
	x_puts("h               : Help");
	x_puts("d  [adr] [len]  : Dump memory");
	x_puts("w  <adr> <dat>  : Write byte");
	x_puts("r  [adr]        : Read byte");
	x_puts("ld <adr>        : Load by XMODEM");
	x_puts("sv <adr> <len>  : Save by XMODEM");
}

//
// Hex dump
//
static void c_dump(token_list *t) {
	static uint8_t *adr = 0;
    unsigned int tmp, len = 16;

	switch (t->n) {
	case 3:
	    if (get_uint(t, T_PARAM2, &len) != 0) {
		    return;     // parameter error
	    }
	case 2:
		if (get_uint(t, T_PARAM1, &tmp) != 0) {
		    return;     // parameter error
	    }
		adr = (uint8_t *)tmp;
	}

	int mod, line = len / 16;
	do {
		if (line > 0) {
			mod = 16;
		} else if ((mod = len % 16) == 0) {
			break;
		}
		
		x_printf("$%04x ", adr);
		for (int i = 0; i < mod; i++) {
			x_printf("%02x ", ((uint8_t *)adr)[i]);
		}
		for (int i = 0; i < 16 - mod; i++) {
			x_printf("   ");
		}
		x_putchar(' ');

		char c;
		for (int i = 0; i < mod; i++) {
			c = *(uint8_t *)adr++;
			if (c < 0x20 || c > 0x7e) {
				c = '.';
			}
			x_putchar(c);
		}
		x_puts("");
	} while (line--);
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
	switch (r_xmodem((uint8_t *)dest, &size)) {
		case -1:
		x_puts("Transfer error");
		break;
		case -2:
		x_puts("Timed out.");
		break;
		case -4:
		x_puts("Copy error.");
		break;
		default:
		x_puts("Success.");
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
	switch (s_xmodem((uint8_t *)src, blks)) {
		case -1:
			x_puts("Transfer error");
			break;
		case -2:
			x_puts("Timed out.");
			break;
		case -3:
			x_puts("Aborted by receiver.");
			break;
		default:
			x_puts("Success.");
			break;
	}
}

//
// Command parser
//
static const struct {
	const char *name;
	void (*func)(token_list *);
} cmd_list[] = {
	{"d",  c_dump},
	{"r",  c_read_byte},
	{"w",  c_write_byte},
	{"ld", c_load},
	{"sv", c_save},
	{"h",  c_help},
	{"",   NULL}
};

static void exec_command(token_list *t) {
	for (int i = 0; cmd_list[i].func != NULL; i++) {
		if (!strcmp(cmd_list[i].name, t->token[T_CMD])) {
			cmd_list[i].func(t);
			break;
		}
	}
}

//
// Monitor main
//
void monitor(void)
{
	token_list tokens;
	char cmd_line[32];

	x_puts("\nATmega128 Tiny Monitor");
	while(1) {
		x_putchar('>');
		x_gets_s(cmd_line, sizeof(cmd_line)-1);
		//x_puts(cmd_line);
		tokenizer(cmd_line, " ", &tokens);
		exec_command(&tokens);
	}
}
