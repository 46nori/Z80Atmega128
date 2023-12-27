/*
 * em_diskio.c
 *
 * Created: 2023/06/28 22:31:49
 *  Author: 46nori
 */ 
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "em_diskio.h"
#include "fatfs/ff.h"
#include "fatfs/diskio.h"
#include "xconsoleio.h"
#include "z80io.h"

#define DEBUG_PRINT_STATE		0
#define DEBUG_PRINT_OPEN		0
#define DEBUG_PRINT_WR			0
#define DEBUG_PRINT_WR_DATA		0
#define DEBUG_PRINT_RD			0

//=================================================================
// DISK I/O emulated device
//=================================================================
// Disk parameters
static uint8_t int_level_write = 128;
static uint8_t int_level_read  = 128;

enum DISKIO_STATUS {IDLE, REQUESTING, DOING, REJECTED};
struct diskio {
	enum DISKIO_STATUS state;
	DWORD	position;
	void	*buffer;
	UINT	length;
	FRESULT	result;
} rd, wr;

static uint8_t tmpbuf[512];
static FRESULT read_result  = FR_OK;
static FRESULT write_result = FR_OK;
FATFS file_system;
#define MAX_FILES	5
struct FD {
	FIL fil;
	FRESULT fr;
	char name[16];
} fd[MAX_FILES];
static struct FD *cfd;

///////////////////////////////////////////////////////////////////
// Initialize emulated device
///////////////////////////////////////////////////////////////////
void init_em_diskio(void)
{
	/* Initialize physical drive */
	DSTATUS status = disk_initialize(0);
	if (status == 0) {
		/* Set SPI clock faster after initialization */
		SPSR |= _BV(SPI2X);		// 1/32 With 16MHz F_CPU
	}
	
	/* Mount volume */
	PORTE &= ~_BV(PORTE5);			// DEBUG: BLUE LED ON PE5
	FRESULT result = f_mount(&file_system, "", 1);
	if (result != FR_OK) {
		x_puts("SDHC mount error");
	} else {
		/* open disk images file */
		for (int i = MAX_FILES-1; i >= 0; i--) {
			cfd = &fd[i];
			sprintf(cfd->name, "DISK%02d.IMG", i);
			cfd->fr = f_open(&cfd->fil, (const char *)&cfd->name, FA_READ | FA_WRITE);
#if DEBUG_PRINT_OPEN
			if (cfd->fr != FR_OK) {
				x_printf("%s open error : %d\n", cfd->name, cfd->fr);
			}
#endif
		}

		rd.state = IDLE;
		wr.state = IDLE;
		PORTE |= _BV(PORTE5);		// DEBUG: BLUE LED OFF PE5
	}
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// CAUTION: Followings are PROHIBITED here
//  * XMEM external SRAM R/W
//  * invoke /BUSREQ
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void OUT_0A_DSK_SelectDisk(uint8_t data)
{
#if DEBUG_PRINT_OPEN
	x_printf("###SELSDK: %d\n", data);
#endif
	if (data < MAX_FILES) {
		cfd = &fd[data];
		rd.state = IDLE;
		wr.state = IDLE;
		read_result  = FR_OK;
		write_result = FR_OK;
	}
}

uint8_t IN_0A_DSK_GetDiskStatus()
{
	if (cfd->fr != FR_OK) {
		return 1;	// error
	}
	return 0;		// success
}

//
//	Setter/Getter of DISK I/O parameters
//
#define QSTRING(str) #str
#define TEMPLATE_IN_OUT_4BYTES(INTNUM,FUNC) \
static int		st_##FUNC = 0;\
static uint32_t	dt_##FUNC = 0;\
uint8_t IN_##INTNUM##_##FUNC(void)\
{\
	st_##FUNC = 0;\
	return dt_##FUNC;\
}\
void OUT_##INTNUM##_##FUNC(uint8_t data)\
{\
	switch (st_##FUNC) {\
	case 0:\
		dt_##FUNC = (uint32_t)data << 24;\
		st_##FUNC = 1;\
		break;\
	case 1:\
		dt_##FUNC |= (uint32_t)data << 16;\
		st_##FUNC = 2;\
		break;\
	case 2:\
		dt_##FUNC |= (uint32_t)data << 8;\
		st_##FUNC = 3;\
		break;\
	case 3:\
		dt_##FUNC |= data;\
	default:\
		st_##FUNC = 0;\
		/*x_printf("   %s=%08lx\n", QSTRING(FUNC), dt_##FUNC);*/ \
		break;\
	}\
}

#define TEMPLATE_IN_OUT_2BYTES(INTNUM,FUNC) \
static int		st_##FUNC = 0;\
static uint16_t	dt_##FUNC = 0;\
uint8_t IN_##INTNUM##_##FUNC(void)\
{\
	st_##FUNC = 0;\
	return dt_##FUNC;\
}\
void OUT_##INTNUM##_##FUNC(uint8_t data)\
{\
	switch (st_##FUNC) {\
	case 0:\
		dt_##FUNC = (uint16_t)data << 8;\
		st_##FUNC = 1;\
		break;\
	case 1:\
		dt_##FUNC |= data;\
	default:\
		st_##FUNC = 0;\
		/*x_printf("   %s=%04x\n", QSTRING(FUNC), dt_##FUNC);*/ \
		break;\
	}\
}

TEMPLATE_IN_OUT_4BYTES(0B,DSK_WritePos)
TEMPLATE_IN_OUT_2BYTES(0C,DSK_WriteBuf)
TEMPLATE_IN_OUT_2BYTES(0D,DSK_WriteLen)
TEMPLATE_IN_OUT_4BYTES(10,DSK_ReadPos)
TEMPLATE_IN_OUT_2BYTES(11,DSK_ReadBuf)
TEMPLATE_IN_OUT_2BYTES(12,DSK_ReadLen)

///////////////////////////////////////////////////////////////////
// DISK WRITE
///////////////////////////////////////////////////////////////////
uint8_t IN_0E_DSK_WriteStatus()
{
	// CAUTION: don't consume long time
	cli();
	uint8_t st = 0x00;
	switch (wr.state) {
		case IDLE:
			if (write_result != FR_OK) {
				st = 0x02;		// error
			}
			break;
		case REQUESTING:
		case DOING:
			st = 0x01;			// writing
			break;
		case REJECTED:
			st = 0x04;			// rejected
			break;
	}
	sei();
	return st;
}

void OUT_0E_DSK_Write(uint8_t data)
{
#if DEBUG_PRINT_STATE
	x_printf("WRITE:%d->", wr.state);
#endif
	// Reject if READ is on going
	if (rd.state == DOING || rd.state == REQUESTING) {
		wr.state = REJECTED;
	} else {
		switch (wr.state) {
		case IDLE:							// 0
		case REJECTED:						// 3
			wr.buffer   = (void *)dt_DSK_WriteBuf;
			wr.length   = dt_DSK_WriteLen;
			wr.position = dt_DSK_WritePos;
			wr.state    = REQUESTING;		// 1
			break;
		case REQUESTING:					// 1
		case DOING:							// 2
			wr.state = REJECTED;			// 3
			break;
		default:
			break;
		}
	}
#if DEBUG_PRINT_STATE
	x_printf("%d\n", wr.state);
#endif
}

uint8_t IN_0F_DSK_WriteIntlevel(void)
{
	return int_level_write;
}

void OUT_0F_DSK_WriteIntLevel(uint8_t data)
{
	int_level_write = data;
}

//
//  Write block
//
//  | : sector(512byte) boundary
//  . : pre-read data
//  # : write data
//
//    sizeof(tmpbuf)
//   <------> <------> <------> <------>
//  |        |        |        |        |
//  |.....#########################.....|
//        ^wr.position
//        <------wr.length-------->
//   <----> offset
//
void em_disk_write(void)
{
	if (wr.state != REQUESTING) {
		return;
	}
	if (rd.state == DOING || rd.state == REQUESTING) {
		wr.state = REJECTED;
		if (int_level_write < 128) {
			// CAUTION: vector is NOT interrupt number(0-127)
			Z80_EXTINT_low(int_level_write << 1);
		}
		return;
	}
	wr.state = DOING;

	UINT bytes;
	unsigned int offset = wr.position % sizeof(tmpbuf);
	void *src = wr.buffer;
	UINT len = wr.length;

	// move to the first sector to write if necessary
	DWORD pos = wr.position;
	if (offset > 0) {
		pos -= offset;			// set pos previous block boundary
	}
	if (pos != f_tell(&cfd->fil)) {
		if ((write_result = f_lseek(&cfd->fil, pos)) != FR_OK) {
			goto error_skip;
		}		
	}

#if DEBUG_PRINT_WR_DATA
	x_puts("");//debug
#endif
	// process the first sector which is unaligned
	if (offset > 0) {
		// read
#if DEBUG_PRINT_WR
		x_printf("$$$ Head Read/");
#endif
		write_result = f_read(&cfd->fil, tmpbuf, sizeof(tmpbuf), &bytes);
		if (write_result != FR_OK) {
			goto error_skip;
		}
		//rewind
		write_result = f_lseek(&cfd->fil, f_tell(&cfd->fil) - sizeof(tmpbuf));
		if (write_result != FR_OK) {
			goto error_skip;
		}
		// modify
#if DEBUG_PRINT_WR
		x_printf("Modify/");
#endif
		unsigned int plen;
		if (offset + len <= sizeof(tmpbuf)) {
			plen = len;
		} else {
			plen = sizeof(tmpbuf) - offset;			
		}
		cli();
		ExtMem_attach();
		memcpy(&tmpbuf[offset], src, plen);
		ExtMem_detach();
		sei();
		// write
#if DEBUG_PRINT_WR
		x_printf("Write\n");
#endif
		if ((write_result = f_write(&cfd->fil, tmpbuf, sizeof(tmpbuf), &bytes)) != FR_OK) {
			goto error_skip;
		}
		src = (uint8_t*)src + plen;
		len -= plen;
#if DEBUG_PRINT_WR_DATA
		for (unsigned int i = offset; i < offset + plen; i++) {
			if (!isprint(tmpbuf[i]) || tmpbuf[i] < ' ') {
				x_putchar('.');
			} else {
				x_putchar(tmpbuf[i]);
			}
		}
		x_puts("");
		for (unsigned int i = offset; i < offset + plen; i++) {
			x_printf("%02x ", tmpbuf[i]);
			if (i % 32 == 31) x_puts("");
		}
		x_puts("");
#endif
	}

	// process the first aligned sector or later sectors
	unsigned int n = len / sizeof(tmpbuf);
	for (unsigned int i = 0; i < n; i++) {
#if DEBUG_PRINT_WR
		x_printf("$$$ %d/%d\n", i, n);
#endif
		cli();
		ExtMem_attach();
		memcpy(tmpbuf, src, sizeof(tmpbuf));
		ExtMem_detach();
		sei();
		if ((write_result = f_write(&cfd->fil, tmpbuf, sizeof(tmpbuf), &bytes)) != FR_OK) {
			goto error_skip;
		}
		src = (uint8_t*)src + sizeof(tmpbuf);
		len -= sizeof(tmpbuf);
#if DEBUG_PRINT_WR_DATA
		for (int i = 0; i < sizeof(tmpbuf); i++) {
			if (!isprint(tmpbuf[i]) || tmpbuf[i] < ' ') {
				x_putchar('.');
			} else {
				x_putchar(tmpbuf[i]);
			}
		}
		x_puts("");
		for (unsigned int i = 0; i < sizeof(tmpbuf); i++) {
			x_printf("%02x ", tmpbuf[i]);
			if (i % 32 == 31) x_puts("");
		}
		x_puts("");
#endif
	}
	if (offset > 0 || n > 0) {
		f_sync(&cfd->fil);
	}

	// process the last sector 
	if (len > 0) {
#if DEBUG_PRINT_WR
		x_printf("$$$ Tail Read/");
#endif
		// read
		write_result = f_read(&cfd->fil, tmpbuf, sizeof(tmpbuf), &bytes);
		if (write_result != FR_OK) {
			goto error_skip;
		}
		// rewind
		write_result = f_lseek(&cfd->fil, f_tell(&cfd->fil) - sizeof(tmpbuf));
		if (write_result != FR_OK) {
			goto error_skip;
		}
		// modify
#if DEBUG_PRINT_WR
		x_printf("Modify/");
#endif
		cli();
		ExtMem_attach();
		memcpy(tmpbuf, src, len);
		ExtMem_detach();
		sei();
		// write
#if DEBUG_PRINT_WR
		x_printf("Write\n");
#endif
		write_result = f_write(&cfd->fil, tmpbuf, sizeof(tmpbuf), &bytes);
		f_sync(&cfd->fil);
#if DEBUG_PRINT_WR_DATA
		for (unsigned int i = 0; i < len; i++) {
			if (!isprint(tmpbuf[i]) || tmpbuf[i] < ' ') {
				x_putchar('.');
			} else {
				x_putchar(tmpbuf[i]);
			}
		}
		x_puts("");
		for (unsigned int i = 0; i < len; i++) {
			x_printf("%02x ", tmpbuf[i]);
			if (i % 32 == 31) x_puts("");
		}
		x_puts("");
#endif
	}

error_skip:
#if DEBUG_PRINT_WR
	x_printf("!!!WRITE:%06lx, %04x : %02x\n\n", wr.position, wr.length, write_result);
	x_puts("-----------");
#endif
	if (int_level_write < 128) {
		// CAUTION: vector is NOT interrupt number(0-127)
		Z80_EXTINT_low(int_level_write << 1);
	}
	wr.state = IDLE;	
}

///////////////////////////////////////////////////////////////////
// DISK READ
///////////////////////////////////////////////////////////////////
uint8_t IN_13_DSK_ReadStatus()
{
	// CAUTION: don't consume long time
	cli();
	uint8_t st = 0x00;
	switch (rd.state) {
	case IDLE:
		if (read_result != FR_OK) {
			st = 0x02;		// error
		}
		break;
	case REQUESTING:
	case DOING:
		st = 0x01;			// reading
		break;
	case REJECTED:
		st = 0x04;			// rejected
		break;
	}
	sei();
	return st;
}

void OUT_13_DSK_Read(uint8_t data)
{
#if DEBUG_PRINT_STATE
	x_printf("READ:%d->", rd.state);
#endif
	// Reject if WRITE is on going
	if (wr.state == DOING || wr.state == REQUESTING) {
		rd.state = REJECTED;
	} else {
		switch (rd.state) {
		case IDLE:						// 0
		case REJECTED:					// 3
			rd.buffer   = (void *)dt_DSK_ReadBuf;
			rd.length   = dt_DSK_ReadLen;
			rd.position = dt_DSK_ReadPos;
			rd.state    = REQUESTING;	// 1
			break;
		case REQUESTING:				// 1
		case DOING:						// 2
			rd.state = REJECTED;		// 3
			break;
		default:
			break;
		}
	}
#if DEBUG_PRINT_STATE
	x_printf("%d\n", rd.state);
#endif
}

uint8_t IN_14_DSK_ReadIntLevel(void)
{
	return int_level_read;
}

void OUT_14_DSK_ReadIntLevel(uint8_t data)
{
	int_level_read = data;
}

void em_disk_read(void)
{
	if (rd.state != REQUESTING) {
		return;
	}
	if (wr.state == DOING || wr.state == REQUESTING) {
		rd.state = REJECTED;
		if (int_level_read < 128) {
			// CAUTION: vector is NOT interrupt number(0-127)
			Z80_EXTINT_low(int_level_read << 1);
		}
		return;
	}
	rd.state = DOING;

	// move to the first sector to read if necessary
	if (f_tell(&cfd->fil) != rd.position) {
		read_result = f_lseek(&cfd->fil, rd.position);
		if (read_result != FR_OK) {
			goto error_skip;
		}
	}

	void *dst = rd.buffer;
	UINT len = rd.length;
	UINT br;

	unsigned int n = len / sizeof(tmpbuf);
	for (unsigned int i = 0; i < n; i++) {
		read_result = f_read(&cfd->fil, tmpbuf, sizeof(tmpbuf), &br);
		if (read_result != FR_OK) {
			goto error_skip;
		}
		cli();
		ExtMem_attach();
		memcpy(dst, tmpbuf, sizeof(tmpbuf));
		ExtMem_detach();
		sei();
		dst = (uint8_t*)dst + sizeof(tmpbuf);
	}
	len = len % sizeof(tmpbuf);
	if (len > 0) {
		read_result = f_read(&cfd->fil, tmpbuf, len, &br);
		if (read_result == FR_OK) {
			cli();
			ExtMem_attach();
			memcpy(dst, tmpbuf, len);
			ExtMem_detach();
			sei();
		}
	}
	
error_skip:
#if DEBUG_PRINT_RD
	x_printf(">>>READ:%06lx, %04x : %02x\n\n", rd.position, rd.length, read_result);
	x_puts("-----------");
#endif
	if (int_level_read < 128) {
		// CAUTION: vector is NOT interrupt number(0-127)
		Z80_EXTINT_low(int_level_read << 1);
	}
	rd.state = IDLE;
}