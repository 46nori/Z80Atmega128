/*
 * intelhex.c
 *
 * Created: 2023/06/18 12:11:18
 *  Author: 46nori
 */ 
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "intelhex.h"

static void init_rbuf(struct ix_rbuf *p) {
	p->index = 0;
	memset(p->buf, '\0', RBUF_SIZE);
}

static int push_rbuf(struct ix_rbuf *p, char c) {
	if (isxdigit(c) && (p->index < RBUF_SIZE)) {
		p->buf[p->index++] = c;
	}
	return p->index;
}

/**
 *    @brief Initialize ix_ctx
 *
 *    @param [in] p : pointer of ix_ctx instance
 *           [in] f : 1 byte memory write function. 
 */
void init_ix(struct ix_ctx *p, MemWriteFunc f) {
	p->func    = f;
	p->state   = HEAD;
	p->length  = 0;
	p->address = 0;
	p->sum     = 0;
	p->status  = 0;
	init_rbuf(&p->rbuf);
}

/**
 *    @brief Set binary converted from INTEX HEX stream
 *
 *    @param [in] p  : pointer of ix_ctx instance
 *           [in] c  : Character of INTEX HEX stream
 *    @retval  0  Success
 *    @retval  1  need next character stream
 *    @retval  2  Check sum error
 *    @retval  3  Sequence error
 *    @retval  4  Internal error
 *
 *    @note RecordType 02 is not supported 
 */
int push_ix(struct ix_ctx *p, char c) {
	int tmp;
	int ret = 1;			// need next data

	switch (p->state) {
	case HEAD:
		if (c == ':') {
			p->state = NUMBER;
			init_rbuf(&p->rbuf);
		} else {
			p->state = DONE;
			ret = 3;		// sequence error
		}
		break;
	case NUMBER:
		if (push_rbuf(&p->rbuf, c) == 2) {
			sscanf(p->rbuf.buf, "%x", &p->length);
			p->state = ADDRESS;
			init_rbuf(&p->rbuf);
		}
		break;
	case ADDRESS:
		if (push_rbuf(&p->rbuf, c) == 4) {
			sscanf(p->rbuf.buf, "%x", &p->address);
			p->state = RECORD;
			init_rbuf(&p->rbuf);
		}
		break;
	case RECORD:
		if (push_rbuf(&p->rbuf, c) == 2) {
			sscanf(p->rbuf.buf, "%x", &tmp);
			if (tmp == 0) {
				p->state = DATA;
			} else {
				p->state = END;
			}
			init_rbuf(&p->rbuf);
		}
		break;
	case DATA:
		if (push_rbuf(&p->rbuf, c) == 2) {
			sscanf(p->rbuf.buf, "%x", &tmp);
			p->sum += tmp;
			p->func((unsigned int)p->address++,
			        (unsigned int)tmp);
			if (--p->length == 0) {
				p->state = SUM;
			}
			init_rbuf(&p->rbuf);
		}
		break;
	case SUM:
		if (push_rbuf(&p->rbuf, c) == 2) {
			sscanf(p->rbuf.buf, "%x", &tmp);
			if (tmp != (p->sum & 0xff)) {
				p->state = DONE;
				ret = 2;		// check sum error
			} else {
				init_ix(p, *p->func);
			}
		}
		break;
	case END:
		if (push_rbuf(&p->rbuf, c) == 2) {
			ret = 0;			// done
			p->state = DONE;
		}
		break;
	case DONE:
		ret = 0;				// done
		break;
	default:
		ret = 4;				// internal error
		break;
	}
	p->status = ret;
	return ret;
}