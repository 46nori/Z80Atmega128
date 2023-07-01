/*
 * emuldev.c
 *
 * Created: 2023/06/28 22:59:18
 *  Author: 46nori
 */ 

#include "emuldev.h"

//
// Initialize emulated device
//
void init_emuldev(void)
{
	init_em_console();
	init_em_diskio();
}

static void OUT_Undefined(uint8_t data) {}
static uint8_t IN_Undefined(void) {return 0;}

///////////////////////////////////////////////////////////////////
// Z80 IN instruction Handler table
///////////////////////////////////////////////////////////////////
uint8_t (* const InHandler[PORT_MAX])(void) = {
	IN_Undefined,
	IN_01_CON_GetInterrupt,
	IN_02_CON_Input,
	IN_03_CON_Status,
	IN_04_DSK_GetDiskNumber,
	IN_05_DSK_WritePos_Low,
	IN_06_DSK_WritePos_Mid,
	IN_07_DSK_WritePos_High,
	IN_08_DSK_WriteBuf_Low,
	IN_09_DSK_WriteBuf_High,
	IN_0A_DSK_WriteLen_Low,
	IN_0B_DSK_WriteLen_High,
	IN_0C_DSK_WriteStatus,
	IN_0D_DSK_ReadPos_Low,
	IN_0E_DSK_ReadPos_Mid,
	IN_0F_DSK_ReadPos_High,
	IN_10_DSK_ReadBuf_Low,
	IN_11_DSK_ReadBuf_High,
	IN_12_DSK_ReadLen_Low,
	IN_13_DSK_ReadLen_High,
	IN_14_DSK_ReadStatus,
	IN_Undefined,
	IN_Undefined,
	IN_Undefined,
	IN_Undefined,
	IN_Undefined,
	IN_Undefined,
	IN_Undefined,
	IN_Undefined,
	IN_Undefined,
	IN_Undefined,
	IN_Undefined
};

///////////////////////////////////////////////////////////////////
// Z80 OUT instruction Handler table
///////////////////////////////////////////////////////////////////
void (* const OutHandler[PORT_MAX])(uint8_t) = {
	OUT_Undefined,
	OUT_01_CON_SetInterrupt,
	OUT_02_CON_Output,
	OUT_03_CON_Flush,
	OUT_04_DSK_SelectDisk,
	OUT_05_DSK_WritePos_Low,
	OUT_06_DSK_WritePos_Mid,
	OUT_07_DSK_WritePos_High,
	OUT_08_DSK_WriteBuf_Low,
	OUT_09_DSK_WriteBuf_High,
	OUT_0A_DSK_WriteLen_Low,
	OUT_0B_DSK_WriteLen_High,
	OUT_0C_DSK_Write,
	OUT_0D_DSK_ReadPos_Low,
	OUT_0E_DSK_ReadPos_Mid,
	OUT_0F_DSK_ReadPos_High,
	OUT_10_DSK_ReadBuf_Low,
	OUT_11_DSK_ReadBuf_High,
	OUT_12_DSK_ReadLen_Low,
	OUT_13_DSK_ReadLen_High,
	OUT_14_DSK_Read,
	OUT_Undefined,
	OUT_Undefined,
	OUT_Undefined,
	OUT_Undefined,
	OUT_Undefined,
	OUT_Undefined,
	OUT_Undefined,
	OUT_Undefined,
	OUT_Undefined,
	OUT_Undefined,
	OUT_Undefined
};
