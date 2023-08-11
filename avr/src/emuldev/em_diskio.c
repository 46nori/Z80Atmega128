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

//=================================================================
// DISK I/O emulated device
//=================================================================
// Disk parameters
static uint8_t disk_status = 0;
static uint8_t write_pos_low  = 0;
static uint8_t write_pos_mid  = 0;
static uint8_t write_pos_high = 0;
static uint8_t write_buf_low  = 0;
static uint8_t write_buf_high = 0;
static uint8_t write_buf_len_low  = 0;
static uint8_t write_buf_len_high = 0;
static uint8_t write_int_level = 128;
static uint8_t read_pos_low  = 0;
static uint8_t read_pos_mid  = 0;
static uint8_t read_pos_high = 0;
static uint8_t read_buf_low  = 0;
static uint8_t read_buf_high = 0;
static uint8_t read_buf_len_low  = 0;
static uint8_t read_buf_len_high = 0;
static uint8_t read_int_level = 128;

enum DISKIO_STATUS {IDLE, REQUESTING, DOING, REJECTED};
static volatile enum DISKIO_STATUS rd_st = IDLE;
static volatile enum DISKIO_STATUS wr_st = IDLE;
static uint8_t tmpbuf[512];

static FRESULT read_result;
static FRESULT write_result;
FATFS file_system;

static const char* get_filename(int disk_no);

#define DEBUG_PRINT 0

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
#if DEBUG_PRINT
//======== for debug ==========================================================
void OUT_0B_DSK_WritePos_Low(uint8_t data)	{write_pos_low  = data;     x_printf("WPOS_L:%02x\n",write_pos_low);}
void OUT_0C_DSK_WritePos_Mid(uint8_t data)	{write_pos_mid  = data;     x_printf("WPOS_M:%02x\n",write_pos_mid);}
void OUT_0D_DSK_WritePos_High(uint8_t data) {write_pos_high = data;     x_printf("WPOS_H:%02x\n",write_pos_high);}
void OUT_0E_DSK_WriteBuf_Low(uint8_t data)	{write_buf_low  = data;     x_printf("WBUF_L:%02x\n",write_buf_low);}
void OUT_0F_DSK_WriteBuf_High(uint8_t data) {write_buf_high = data;     x_printf("WBUF_H:%02x\n",write_buf_high);}
void OUT_10_DSK_WriteLen_Low(uint8_t data)	{write_buf_len_low  = data; x_printf("WLEN_L:%02x\n",write_buf_len_low);}
void OUT_11_DSK_WriteLen_High(uint8_t data)	{write_buf_len_high = data; x_printf("WLEN_H:%02x\n",write_buf_len_high);}
void OUT_14_DSK_ReadPos_Low(uint8_t data)	{read_pos_low = data;       x_printf("RPOS_L:%02x\n",read_pos_low);}
void OUT_15_DSK_ReadPos_Mid(uint8_t data)	{read_pos_mid = data;       x_printf("RPOS_M:%02x\n",read_pos_mid);}
void OUT_16_DSK_ReadPos_High(uint8_t data)	{read_pos_high = data;      x_printf("RPOS_H:%02x\n",read_pos_high);}
void OUT_17_DSK_ReadBuf_Low(uint8_t data)	{read_buf_low  = data;      x_printf("RBUF_L:%02x\n",read_buf_low);}
void OUT_18_DSK_ReadBuf_High(uint8_t data)	{read_buf_high = data;      x_printf("RBUF_H:%02x\n",read_buf_high);}
void OUT_19_DSK_ReadLen_Low(uint8_t data)	{read_buf_len_low  = data;  x_printf("RLEN_L:%02x\n",read_buf_len_low);}
void OUT_1A_DSK_ReadLen_High(uint8_t data)	{read_buf_len_high = data;  x_printf("RLEN_H:%02x\n",read_buf_len_high);}
uint8_t IN_0B_DSK_WritePos_Low()	{return write_pos_low;}
uint8_t IN_0C_DSK_WritePos_Mid()	{return write_pos_mid;}
uint8_t IN_0D_DSK_WritePos_High()	{return write_pos_high;}
uint8_t IN_0E_DSK_WriteBuf_Low()	{return write_buf_low;}
uint8_t IN_0F_DSK_WriteBuf_High()	{return write_buf_high;}
uint8_t IN_10_DSK_WriteLen_Low()	{return write_buf_len_low;}
uint8_t IN_11_DSK_WriteLen_High()	{return write_buf_len_high;}
uint8_t IN_14_DSK_ReadPos_Low()		{return read_pos_low;}
uint8_t IN_15_DSK_ReadPos_Mid()		{return read_pos_mid;}
uint8_t IN_16_DSK_ReadPos_High()	{return read_pos_high;}
uint8_t IN_17_DSK_ReadBuf_Low()		{return read_buf_low;}
uint8_t IN_18_DSK_ReadBuf_High()	{return read_buf_high;}
uint8_t IN_19_DSK_ReadLen_Low()		{return read_buf_len_low;}
uint8_t IN_1A_DSK_ReadLen_High()	{return read_buf_len_high;}
#else
void OUT_0B_DSK_WritePos_Low(uint8_t data)	{write_pos_low  = data;}
void OUT_0C_DSK_WritePos_Mid(uint8_t data)	{write_pos_mid  = data;}
void OUT_0D_DSK_WritePos_High(uint8_t data) {write_pos_high = data;}
void OUT_0E_DSK_WriteBuf_Low(uint8_t data)	{write_buf_low  = data;}
void OUT_0F_DSK_WriteBuf_High(uint8_t data) {write_buf_high = data;}
void OUT_10_DSK_WriteLen_Low(uint8_t data)	{write_buf_len_low  = data;}
void OUT_11_DSK_WriteLen_High(uint8_t data)	{write_buf_len_high = data;}
void OUT_14_DSK_ReadPos_Low(uint8_t data)	{read_pos_low = data;}
void OUT_15_DSK_ReadPos_Mid(uint8_t data)	{read_pos_mid = data;}
void OUT_16_DSK_ReadPos_High(uint8_t data)	{read_pos_high = data;}
void OUT_17_DSK_ReadBuf_Low(uint8_t data)	{read_buf_low  = data;}
void OUT_18_DSK_ReadBuf_High(uint8_t data)	{read_buf_high = data;}
void OUT_19_DSK_ReadLen_Low(uint8_t data)	{read_buf_len_low  = data;}
void OUT_1A_DSK_ReadLen_High(uint8_t data)	{read_buf_len_high = data;}
uint8_t IN_0B_DSK_WritePos_Low()	{return write_pos_low;}
uint8_t IN_0C_DSK_WritePos_Mid()	{return write_pos_mid;}
uint8_t IN_0D_DSK_WritePos_High()	{return write_pos_high;}
uint8_t IN_0E_DSK_WriteBuf_Low()	{return write_buf_low;}
uint8_t IN_0F_DSK_WriteBuf_High()	{return write_buf_high;}
uint8_t IN_10_DSK_WriteLen_Low()	{return write_buf_len_low;}
uint8_t IN_11_DSK_WriteLen_High()	{return write_buf_len_high;}
uint8_t IN_14_DSK_ReadPos_Low()		{return read_pos_low;}
uint8_t IN_15_DSK_ReadPos_Mid()		{return read_pos_mid;}
uint8_t IN_16_DSK_ReadPos_High()	{return read_pos_high;}
uint8_t IN_17_DSK_ReadBuf_Low()		{return read_buf_low;}
uint8_t IN_18_DSK_ReadBuf_High()	{return read_buf_high;}
uint8_t IN_19_DSK_ReadLen_Low()		{return read_buf_len_low;}
uint8_t IN_1A_DSK_ReadLen_High()	{return read_buf_len_high;}
#endif

///////////////////////////////////////////////////////////////////
// DISK WRITE
///////////////////////////////////////////////////////////////////
void OUT_0A_DSK_SelectDisk(uint8_t data)
{
	// Open file
	const char *filename = get_filename(data);
	if (pf_open(filename) != FR_OK) {
		disk_status = 1;		// error
#if DEBUG_PRINT
		x_printf("%s open error\n");
#endif
		return;
	}	
	// Set file pointer to beginning of file
	pf_lseek(0);
	disk_status = 0;			// success
#if DEBUG_PRINT
	x_printf("###SELSDK: %s\n", filename);
#endif
}
uint8_t IN_0A_DSK_GetDiskStatus()
{
	return disk_status;
}

void OUT_12_DSK_Write(uint8_t data)
{
	// CAUTION: don't consume long time
	// Reject if READ is on going
	if (rd_st == DOING || rd_st == REQUESTING) {
		wr_st = REJECTED;
		return;
	}
	switch (wr_st) {
	case IDLE:
	case REJECTED:
		wr_st = REQUESTING;
		break;
	case REQUESTING:
	case DOING:
		wr_st = REJECTED;
		break;
	default:
		break;
	}
}

uint8_t IN_12_DSK_WriteStatus()
{
	// CAUTION: don't consume long time
	cli();
	uint8_t st = 0x00;
	switch (rd_st) {
	case IDLE:
		if (read_result != FR_OK) {
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

void OUT_13_DSK_WriteIntLevel(uint8_t data)
{
	write_int_level = data;
}

uint8_t IN_13_DSK_WriteIntlevel(void)
{
	return write_int_level;
}

void em_disk_write(void)
{
	if (wr_st != REQUESTING) {
		return;
	}
	if (rd_st == DOING || rd_st == REQUESTING) {
		wr_st = REJECTED;
		return;
	}
	wr_st = DOING;

	DWORD pos = (DWORD)write_pos_high << 16 |
	            (DWORD)write_pos_mid  << 8  |
	            (DWORD)write_pos_low;
	pf_lseek(pos);

	void *buf = (void *)((uint16_t)write_buf_high << 8 | write_buf_low);
	UINT len = (UINT)write_buf_len_high << 8 | write_buf_len_low;
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
	x_printf(">>>WRITE:%06lx : %02x\n\n", pos, write_result);

	if (write_int_level < 128) {
		// CAUTION: vector is NOT interrupt number(0-127)
		Z80_EXTINT_low(write_int_level << 1);
	}
	wr_st = IDLE;
}

///////////////////////////////////////////////////////////////////
// DISK READ
///////////////////////////////////////////////////////////////////
void OUT_1B_DSK_Read(uint8_t data)
{
#if DEBUG_PRINT
	x_printf("READ:%d -> ", rd_st);
#endif
	// Reject if WRITE is on going
	if (wr_st == DOING || wr_st == REQUESTING) {
		rd_st = REJECTED;
	} else {
		switch (rd_st) {
		case IDLE:
		case REJECTED:
			rd_st = REQUESTING;
			break;
		case REQUESTING:
		case DOING:
			rd_st = REJECTED;
			break;
		default:
			break;
		}
	}
#if DEBUG_PRINT
	x_printf("%d\n", rd_st);
#endif
}

uint8_t IN_1B_DSK_ReadStatus()
{
	// CAUTION: don't consume long time
	cli();
	uint8_t st = 0x00;
	switch (rd_st) {
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

void OUT_1C_DSK_ReadIntLevel(uint8_t data)
{
	read_int_level = data;
}

uint8_t IN_1C_DSK_ReadIntLevel(void)
{
	return read_int_level;	
}

void em_disk_read(void)
{
	if (rd_st != REQUESTING) {
		return;
	}
	if (wr_st == DOING || wr_st == REQUESTING) {
		rd_st = REJECTED;
		return;
	}
	rd_st = DOING;

	// CAUTION: don't consume long time
	DWORD pos = (DWORD)read_pos_high << 16 |
				(DWORD)read_pos_mid  << 8  |
				(DWORD)read_pos_low;
	pf_lseek(pos);

	void *buf = (void *)((uint16_t)read_buf_high << 8 | read_buf_low);
	UINT len = (UINT)read_buf_len_high << 8 | read_buf_len_low;
	UINT br;

	for (unsigned int i = 0; i < len / sizeof(tmpbuf); i++) {
		if ((read_result = pf_read(tmpbuf, sizeof(tmpbuf), &br)) != FR_OK) {
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
		if ((read_result = pf_read(tmpbuf, len, &br)) == FR_OK) {
			cli();
			ExtMem_attach();
			memcpy(buf, tmpbuf, len);
			ExtMem_detach();
			sei();
		}
	}
	
error_skip:
#if DEBUG_PRINT
	x_printf(">>>READ:%06lx : %02x\n\n", pos, read_result);
#endif
	if (read_int_level < 128) {
		// CAUTION: vector is NOT interrupt number(0-127)
		Z80_EXTINT_low(read_int_level << 1);
	}
	rd_st = IDLE;
}