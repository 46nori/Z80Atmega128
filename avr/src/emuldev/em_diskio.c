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

static uint8_t tmpbuf[512];

// FatFs
static FATFS file_system;

enum DISKIO_STATUS {IDLE, REQUESTING, DOING, REJECTED};
struct diskio {
	enum DISKIO_STATUS state;
	DWORD	position;
	void	*buffer;
	UINT	length;
	FRESULT	result;
};

struct FD {
	FIL fil;
	FRESULT open_result;
	char name[16];
	struct diskio read;
	struct diskio write;
};

#define MAX_FILES	5			// Max number of disk images (A: - E:)
static struct FD fd[MAX_FILES];	// List of disk image files
static struct FD *cfd;			// Current selected disk image file

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
	PORTE &= ~_BV(PORTE5);			// BLUE LED ON PE5
	if (f_mount(&file_system, "", 1) != FR_OK) {
		x_puts("SDHC mount error");
	} else {
		/* open all disk images file in advance */
		for (int i = MAX_FILES-1; i >= 0; i--) {
			cfd = &fd[i];
			sprintf(cfd->name, "DISK%02d.IMG", i);
			cfd->open_result = f_open(&cfd->fil, (const char *)&cfd->name, FA_READ | FA_WRITE);
#if DEBUG_PRINT_OPEN
			if (cfd->open_result != FR_OK) {
				x_printf("%s open error : %d\n", cfd->name, cfd->open_result);
			}
#endif
			cfd->read.state   = IDLE;
			cfd->read.result  = FR_OK;
			cfd->write.state  = IDLE;
			cfd->write.result = FR_OK;
		}
		PORTE |= _BV(PORTE5);		// BLUE LED OFF PE5
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
		// Re-open if previous operation was failure.
		if (cfd->open_result  != FR_OK ||
		    cfd->write.result != FR_OK ||
			cfd->read.result  != FR_OK) {
#if DEBUG_PRINT_OPEN
			x_printf("   Result op:%d/rd:%d/wr:%d\n", cfd->open_result, cfd->read.result, cfd->write.result);
#endif
			f_close(&cfd->fil);
			cfd->open_result = f_open(&cfd->fil, (const char *)&cfd->name, FA_READ | FA_WRITE);
#if DEBUG_PRINT_OPEN
			x_printf("   Re-open: %d\n", cfd->open_result);
#endif
		}
		cfd->read.state   = IDLE;
		cfd->read.result  = FR_OK;
		cfd->write.state  = IDLE;
		cfd->write.result = FR_OK;
	}
}

uint8_t IN_0A_DSK_GetDiskStatus()
{
	if (cfd->open_result != FR_OK) {
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
	switch (cfd->write.state) {
		case IDLE:
			if (cfd->write.result != FR_OK) {
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
	x_printf("WRITE:%d->", cfd->write.state);
#endif
	// Reject if READ is on going
	if (cfd->read.state == DOING || cfd->read.state == REQUESTING) {
		cfd->write.state = REJECTED;
	} else {
		switch (cfd->write.state) {
		case IDLE:								// 0
		case REJECTED:							// 3
			cfd->write.buffer   = (void *)dt_DSK_WriteBuf;
			cfd->write.length   = dt_DSK_WriteLen;
			cfd->write.position = dt_DSK_WritePos;
			cfd->write.state    = REQUESTING;	// 1
			break;
		case REQUESTING:						// 1
		case DOING:								// 2
			cfd->write.state = REJECTED;		// 3
			break;
		default:
			break;
		}
	}
#if DEBUG_PRINT_STATE
	x_printf("%d\n", cfd->write.state);
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
	if (cfd->write.state != REQUESTING) {
		return;
	}
	if (cfd->read.state == DOING || cfd->read.state == REQUESTING) {
		cfd->write.state = REJECTED;
		if (int_level_write < 128) {
			// CAUTION: vector is NOT interrupt number(0-127)
			Z80_EXTINT_low(int_level_write << 1);
		}
		return;
	}
	cfd->write.state = DOING;

	UINT bytes;
	unsigned int offset = cfd->write.position % sizeof(tmpbuf);
	void *src = cfd->write.buffer;
	UINT len = cfd->write.length;

	// move to the first sector to write if necessary
	DWORD pos = cfd->write.position;
	if (offset > 0) {
		pos -= offset;			// set pos previous block boundary
	}
	if (pos != f_tell(&cfd->fil)) {
		if ((cfd->write.result = f_lseek(&cfd->fil, pos)) != FR_OK) {
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
		cfd->write.result = f_read(&cfd->fil, tmpbuf, sizeof(tmpbuf), &bytes);
		if (cfd->write.result != FR_OK) {
			goto error_skip;
		}
		//rewind
		cfd->write.result = f_lseek(&cfd->fil, f_tell(&cfd->fil) - sizeof(tmpbuf));
		if (cfd->write.result != FR_OK) {
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
		if ((cfd->write.result = f_write(&cfd->fil, tmpbuf, sizeof(tmpbuf), &bytes)) != FR_OK) {
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
		if ((cfd->write.result = f_write(&cfd->fil, tmpbuf, sizeof(tmpbuf), &bytes)) != FR_OK) {
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
		cfd->write.result = f_read(&cfd->fil, tmpbuf, sizeof(tmpbuf), &bytes);
		if (cfd->write.result != FR_OK) {
			goto error_skip;
		}
		// rewind
		cfd->write.result = f_lseek(&cfd->fil, f_tell(&cfd->fil) - sizeof(tmpbuf));
		if (cfd->write.result != FR_OK) {
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
		cfd->write.result = f_write(&cfd->fil, tmpbuf, sizeof(tmpbuf), &bytes);
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
	x_printf("!!!WRITE:%06lx, %04x : %02x\n\n", cfd->write.position, cfd->write.length, cfd->write.result);
	x_puts("-----------");
#endif
	if (int_level_write < 128) {
		// CAUTION: vector is NOT interrupt number(0-127)
		Z80_EXTINT_low(int_level_write << 1);
	}
	cfd->write.state = IDLE;	
}

///////////////////////////////////////////////////////////////////
// DISK READ
///////////////////////////////////////////////////////////////////
uint8_t IN_13_DSK_ReadStatus()
{
	// CAUTION: don't consume long time
	cli();
	uint8_t st = 0x00;
	switch (cfd->read.state) {
	case IDLE:
		if (cfd->read.result != FR_OK) {
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
	x_printf("READ:%d->", cfd->read.state);
#endif
	// Reject if WRITE is on going
	if (cfd->write.state == DOING || cfd->write.state == REQUESTING) {
		cfd->read.state = REJECTED;
	} else {
		switch (cfd->read.state) {
		case IDLE:								// 0
		case REJECTED:							// 3
			cfd->read.buffer   = (void *)dt_DSK_ReadBuf;
			cfd->read.length   = dt_DSK_ReadLen;
			cfd->read.position = dt_DSK_ReadPos;
			cfd->read.state    = REQUESTING;	// 1
			break;
		case REQUESTING:						// 1
		case DOING:								// 2
			cfd->read.state = REJECTED;			// 3
			break;
		default:
			break;
		}
	}
#if DEBUG_PRINT_STATE
	x_printf("%d\n", cfd->read.state);
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
	if (cfd->read.state != REQUESTING) {
		return;
	}
	if (cfd->write.state == DOING || cfd->write.state == REQUESTING) {
		cfd->read.state = REJECTED;
		if (int_level_read < 128) {
			// CAUTION: vector is NOT interrupt number(0-127)
			Z80_EXTINT_low(int_level_read << 1);
		}
		return;
	}
	cfd->read.state = DOING;

	// move to the first sector to read if necessary
	if (f_tell(&cfd->fil) != cfd->read.position) {
		cfd->read.result = f_lseek(&cfd->fil, cfd->read.position);
		if (cfd->read.result != FR_OK) {
			goto error_skip;
		}
	}

	void *dst = cfd->read.buffer;
	UINT len = cfd->read.length;
	UINT br;

	unsigned int n = len / sizeof(tmpbuf);
	for (unsigned int i = 0; i < n; i++) {
		cfd->read.result = f_read(&cfd->fil, tmpbuf, sizeof(tmpbuf), &br);
		if (cfd->read.result != FR_OK) {
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
		cfd->read.result = f_read(&cfd->fil, tmpbuf, len, &br);
		if (cfd->read.result == FR_OK) {
			cli();
			ExtMem_attach();
			memcpy(dst, tmpbuf, len);
			ExtMem_detach();
			sei();
		}
	}
	
error_skip:
#if DEBUG_PRINT_RD
	x_printf(">>>READ:%06lx, %04x : %02x\n\n", cfd->read.position, cfd->read.length, cfd->read.result);
	x_puts("-----------");
#endif
	if (int_level_read < 128) {
		// CAUTION: vector is NOT interrupt number(0-127)
		Z80_EXTINT_low(int_level_read << 1);
	}
	cfd->read.state = IDLE;
}