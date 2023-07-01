/*
 * emuldev.h
 *
 * Created: 2023/06/28 22:58:51
 *  Author: 46nori
 */ 

#ifndef EMULDEV_H_
#define EMULDEV_H_
#include "asf.h"
#include "em_consoleio.h"
#include "em_diskio.h"

#define PORT_MAX 32
extern uint8_t (* const InHandler[PORT_MAX])(void);
extern void (* const OutHandler[PORT_MAX])(uint8_t);

extern void init_emuldev(void);

#endif /* EMULDEV_H_ */