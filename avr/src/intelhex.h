/*
 * intelhex.h
 *
 * Created: 2023/06/18 12:12:31
 *  Author: 46nori
 */ 

#ifndef INTELHEX_H_
#define INTELHEX_H_

typedef int (*MemWriteFunc)(unsigned int, unsigned int);

#define RBUF_SIZE 5
struct ix_rbuf {
	char buf[RBUF_SIZE];
	int index;
};

enum ix_state {
	HEAD, NUMBER, ADDRESS, RECORD, DATA, SUM, END, DONE
};
struct ix_ctx {
	MemWriteFunc func;
	struct ix_rbuf rbuf;
	enum ix_state state;
	int length;
	int address;
	int sum;
	int status;
};

void init_ix(struct ix_ctx *p, MemWriteFunc f);
int push_ix(struct ix_ctx *p, char c);

#endif /* INTELHEX_H_ */