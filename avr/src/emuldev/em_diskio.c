/*
 * em_diskio.c
 *
 * Created: 2023/06/28 22:31:49
 *  Author: 46nori
 */ 
#include "stdio.h"
#include "em_diskio.h"
#include "petitfs/pff.h"
#include "petitfs/diskio.h"
#include "xconsoleio.h"
#include "z80io.h"

static volatile uint8_t z80_int_num_write;
static volatile uint8_t z80_int_num_read;
FATFS file_system;
static const char* get_filename(int disk_no);

//
// Initialize emulated device
//
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

//=================================================================
// DISK I/O emulated device
//=================================================================
// Disk parameters
static uint8_t disk_no = 0;
static uint8_t write_pos_low  = 0;
static uint8_t write_pos_mid  = 0;
static uint8_t write_pos_high = 0;
static uint8_t write_buf_low  = 0;
static uint8_t write_buf_high = 0;
static uint8_t write_buf_len_low  = 0;
static uint8_t write_buf_len_high = 0;
static uint8_t read_pos_low  = 0;
static uint8_t read_pos_mid  = 0;
static uint8_t read_pos_high = 0;
static uint8_t read_buf_low  = 0;
static uint8_t read_buf_high = 0;
static uint8_t read_buf_len_low  = 0;
static uint8_t read_buf_len_high = 0;

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// CAUTION: Followings are PROHIBITED here
//  * XMEM external SRAM R/W
//  * invoke /BUSREQ
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void OUT_05_DSK_WritePos_Low(uint8_t data)	{write_pos_low  = data;}
void OUT_06_DSK_WritePos_Mid(uint8_t data)	{write_pos_mid  = data;}
void OUT_07_DSK_WritePos_High(uint8_t data) {write_pos_high = data;}
void OUT_08_DSK_WriteBuf_Low(uint8_t data)	{write_buf_low  = data;}
void OUT_09_DSK_WriteBuf_High(uint8_t data) {write_buf_high = data;}
void OUT_0A_DSK_WriteLen_Low(uint8_t data)	{write_buf_len_low  = data;}
void OUT_0B_DSK_WriteLen_High(uint8_t data)	{write_buf_len_high = data;}
void OUT_0D_DSK_ReadPos_Low(uint8_t data)	{read_pos_low = data;}
void OUT_0E_DSK_ReadPos_Mid(uint8_t data)	{read_pos_mid = data;}
void OUT_0F_DSK_ReadPos_High(uint8_t data)	{read_pos_high = data;}
void OUT_10_DSK_ReadBuf_Low(uint8_t data)	{read_buf_low  = data;}
void OUT_11_DSK_ReadBuf_High(uint8_t data)	{read_buf_high = data;}
void OUT_12_DSK_ReadLen_Low(uint8_t data)	{read_buf_len_low  = data;}
void OUT_13_DSK_ReadLen_High(uint8_t data)	{read_buf_len_high = data;}
uint8_t IN_05_DSK_WritePos_Low()	{return write_pos_low;}
uint8_t IN_06_DSK_WritePos_Mid()	{return write_pos_mid;}
uint8_t IN_07_DSK_WritePos_High()	{return write_pos_high;}
uint8_t IN_08_DSK_WriteBuf_Low()	{return write_buf_low;}
uint8_t IN_09_DSK_WriteBuf_High()	{return write_buf_high;}
uint8_t IN_0A_DSK_WriteLen_Low()	{return write_buf_len_low;}
uint8_t IN_0B_DSK_WriteLen_High()	{return write_buf_len_high;}
uint8_t IN_0D_DSK_ReadPos_Low()		{return read_pos_low;}
uint8_t IN_0E_DSK_ReadPos_Mid()		{return read_pos_mid;}
uint8_t IN_0F_DSK_ReadPos_High()	{return read_pos_high;}
uint8_t IN_10_DSK_ReadBuf_Low()		{return read_buf_low;}
uint8_t IN_11_DSK_ReadBuf_High()	{return read_buf_high;}
uint8_t IN_12_DSK_ReadLen_Low()		{return read_buf_len_low;}
uint8_t IN_13_DSK_ReadLen_High()	{return read_buf_len_high;}

//
// Port 04H
//
void OUT_04_DSK_SelectDisk(uint8_t data)
{
	disk_no = data % 16;

	/* Open file */
	const char *filename = get_filename(disk_no);
	if (pf_open(filename) != FR_OK) {
		x_puts("SDHC open error");
		return;
	}	
	/* Set file pointer to beginning of file */
	pf_lseek(0);
}
uint8_t IN_04_DSK_GetDiskNumber()
{
	return disk_no;
}

//
// Port 0CH
//
FRESULT write_result;
void OUT_0C_DSK_Write(uint8_t data)
{
	z80_int_num_write = data;

	// CAUTION: don't consume long time	
	DWORD pos = (DWORD)write_pos_high << 16 | write_pos_mid << 8 | write_pos_low;
	pf_lseek(pos);

	void *buf = (void *)(write_buf_high << 8 | write_buf_low);
	UINT len = write_buf_len_high << 8 | write_buf_len_low;
	UINT bw;
	write_result = pf_write(buf, len, &bw);
	if (write_result == FR_OK) {
		// Interrupt
	}
}
uint8_t IN_0C_DSK_WriteStatus()
{
	// CAUTION: don't consume long time
	if (write_result != FR_OK) {
		return 0x02;
	}
	return 0x00;
}

//
// Port 14H
//
FRESULT read_result;
void OUT_14_DSK_Read(uint8_t data)
{
	z80_int_num_read = data;

	// CAUTION: don't consume long time	
	DWORD pos = (DWORD)read_pos_high << 16 | read_pos_mid << 8 | read_pos_low;
	pf_lseek(pos);

	void *buf = (void *)(read_buf_high << 8 | read_buf_low);
	UINT len = read_buf_len_high << 8 | read_buf_len_low;
	UINT br;
	read_result = pf_read(buf, len, &br);
	if (read_result == FR_OK) {
		// Interrupt
	}
}

uint8_t IN_14_DSK_ReadStatus()
{
	// CAUTION: don't consume long time
	if (read_result != FR_OK) {
		return 0x02;
	}
	return 0x00;
}
