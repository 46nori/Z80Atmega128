/*
 * em_diskio.c
 *
 * Created: 2023/06/28 22:31:49
 *  Author: 46nori
 */ 
#include <stdio.h>
#include <string.h>
#include "em_diskio.h"
#include "petitfs/pff.h"
#include "petitfs/diskio.h"
#include "xconsoleio.h"
#include "z80io.h"

#define DEBUG_PRINT 0

//=================================================================
// DISK I/O emulated device
//=================================================================
// Disk parameters
static uint8_t disk_result = 0;
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

static const char* get_filename(int disk_no);

///////////////////////////////////////////////////////////////////
// Initialize emulated device
///////////////////////////////////////////////////////////////////
void init_em_diskio(void)
{
	/* Initialize physical drive */
	DSTATUS status = disk_initialize();
	if (status == 0) {
		/* Set SPI clock faster after initialization */
		SPSR |= _BV(SPI2X);		// 1/32 With 16MHz F_CPU
	}
	
	/* Mount volume */
	FRESULT result = pf_mount(&file_system);
	if (result != FR_OK) {
		x_puts("SDHC mount error");
	}

	// set filename of default disk image
	get_filename(0);

	rd.state = IDLE;
	wr.state = IDLE;
}

static const char* get_filename(int disk_no)
{
	static char *filename = "DISK00.IMG      ";
	sprintf(filename, "DISK%02d.IMG", disk_no);
	return filename;
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// CAUTION: Followings are PROHIBITED here
//  * XMEM external SRAM R/W
//  * invoke /BUSREQ
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void OUT_0A_DSK_SelectDisk(uint8_t data)
{
	// Open file
	const char *filename = get_filename(data);
	if (pf_open(filename) != FR_OK) {
		disk_result = 1;		// error
#if DEBUG_PRINT
		x_printf("%s open error\n", filename);
#endif
		return;
	}	
	// Set file pointer to beginning of file
	if (pf_lseek(0) != FR_OK) {
		disk_result = 1;		// error
#if DEBUG_PRINT
		x_puts("seek error");
#endif
		return;
	}
	disk_result = 0;			// success

	rd.state = IDLE;
	wr.state = IDLE;
	read_result  = FR_OK;
	write_result = FR_OK;

#if DEBUG_PRINT
	x_printf("###SELSDK: %s\n", filename);
#endif
}

uint8_t IN_0A_DSK_GetDiskStatus()
{
	return disk_result;
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
		/*x_printf("%s=%08x\n", QSTRING(FUNC), dt_##FUNC);*/ \
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
		/*x_printf("%s=%04x\n", QSTRING(FUNC), dt_##FUNC);*/ \
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
void OUT_0E_DSK_Write(uint8_t data)
{
#if DEBUG_PRINT
	x_printf("WRITE:%d->", wr.state);
#endif
	// Reject if READ is on going
	if (rd.state == DOING || rd.state == REQUESTING) {
		wr.state = REJECTED;
	} else {
		switch (wr.state) {
		case IDLE:
		case REJECTED:
			wr.buffer   = (void *)dt_DSK_WriteBuf;
			wr.length   = dt_DSK_WriteLen;
			wr.position = dt_DSK_WritePos;
			wr.state    = REQUESTING;
			break;
		case REQUESTING:
		case DOING:
			wr.state = REJECTED;
			break;
		default:
			break;
		}
	}
#if DEBUG_PRINT
	x_printf("%d\n", wr.state);
#endif
}

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

void OUT_0F_DSK_WriteIntLevel(uint8_t data)
{
	int_level_write = data;
}

uint8_t IN_0F_DSK_WriteIntlevel(void)
{
	return int_level_write;
}

void em_disk_write(void)
{
	if (wr.state != REQUESTING) {
		return;
	}
	if (rd.state == DOING || rd.state == REQUESTING) {
		wr.state = REJECTED;
		return;
	}
	wr.state = DOING;

	write_result = pf_lseek(wr.position);
	if (write_result != FR_OK) {
		goto error_skip;
	}

	void *buf = wr.buffer;
	UINT len = wr.length;
	UINT bw;

	for (unsigned int i = 0; i < len / sizeof(tmpbuf); i++) {
		cli();
		ExtMem_attach();
		memcpy(tmpbuf, buf, sizeof(tmpbuf));
		ExtMem_detach();
		sei();
		if ((write_result = pf_write(tmpbuf, sizeof(tmpbuf), &bw)) != FR_OK) {
			goto error_skip;
		}
		buf = (uint8_t*)buf + sizeof(tmpbuf);
	}
	len = len % sizeof(tmpbuf);
	if (len > 0) {
		cli();
		ExtMem_attach();
		memcpy(tmpbuf, buf, len);
		ExtMem_detach();
		sei();
		write_result = pf_write(tmpbuf, len, &bw);
	}

error_skip:
#if DEBUG_PRINT
	x_printf(">>>WRITE:%06lx : %02x\n\n", wr.position, write_result);
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
#if DEBUG_PRINT
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
#if DEBUG_PRINT
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
		return;
	}
	rd.state = DOING;

	// CAUTION: don't consume long time
	read_result = pf_lseek(rd.position);
	if (read_result != FR_OK) {
		goto error_skip;
	}

	void *buf = (void *)rd.buffer;
	UINT len = rd.length;
	UINT br;

	for (unsigned int i = 0; i < len / sizeof(tmpbuf); i++) {
		read_result = pf_read(tmpbuf, sizeof(tmpbuf), &br);
		if (read_result != FR_OK) {
			goto error_skip;
		}
		cli();
		ExtMem_attach();
		memcpy(buf, tmpbuf, sizeof(tmpbuf));
		ExtMem_detach();
		sei();
		buf = (uint8_t*)buf + sizeof(tmpbuf);
	}
	len = len % sizeof(tmpbuf);
	if (len > 0) {
		read_result = pf_read(tmpbuf, len, &br);
		if (read_result == FR_OK) {
			cli();
			ExtMem_attach();
			memcpy(buf, tmpbuf, len);
			ExtMem_detach();
			sei();
		}
	}
	
error_skip:
#if DEBUG_PRINT
	x_printf(">>>READ:%06lx : %02x\n\n", rd.position, read_result);
#endif
	if (int_level_read < 128) {
		// CAUTION: vector is NOT interrupt number(0-127)
		Z80_EXTINT_low(int_level_read << 1);
	}
	rd.state = IDLE;
}