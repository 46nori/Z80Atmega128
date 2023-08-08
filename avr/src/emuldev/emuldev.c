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
	init_em_debugger();
	init_em_led();
	init_em_console();
	init_em_diskio();
}

static void OUT_Undefined(uint8_t data) {}
static uint8_t IN_Undefined(void) {return 0;}

///////////////////////////////////////////////////////////////////
// Z80 IN instruction Handler table
///////////////////////////////////////////////////////////////////
uint8_t (* const InHandler[PORT_MAX])(void) = {
	IN_00_CONIN,
	IN_01_CONIN_GetStatus,
	IN_02_CONIN_GetBufferSize,
	IN_03_CONIN_GetIntLevel,
	IN_Undefined,
	IN_Undefined,
	IN_06_CONOUT_GetStatus,
	IN_07_CONOUT_GetBufferSize,
	IN_08_CONOUT_GetIntLevel,
	IN_Undefined,
	IN_0A_DSK_GetDiskStatus,
	IN_0B_DSK_WritePos_Low,
	IN_0C_DSK_WritePos_Mid,
	IN_0D_DSK_WritePos_High,
	IN_0E_DSK_WriteBuf_Low,
	IN_0F_DSK_WriteBuf_High,
	IN_10_DSK_WriteLen_Low,
	IN_11_DSK_WriteLen_High,
	IN_12_DSK_WriteStatus,
	IN_13_DSK_WriteIntlevel,
	IN_14_DSK_ReadPos_Low,
	IN_15_DSK_ReadPos_Mid,
	IN_16_DSK_ReadPos_High,
	IN_17_DSK_ReadBuf_Low,
	IN_18_DSK_ReadBuf_High,
	IN_19_DSK_ReadLen_Low,
	IN_1A_DSK_ReadLen_High,
	IN_1B_DSK_ReadStatus,
	IN_1C_DSK_ReadIntLevel,
	IN_1D_DEBUG_IntLevel,
	IN_1E_DEBUG_BreakPointAddress,
	IN_1F_LED_Get
};

///////////////////////////////////////////////////////////////////
// Z80 OUT instruction Handler table
///////////////////////////////////////////////////////////////////
void (* const OutHandler[PORT_MAX])(uint8_t) = {
	OUT_Undefined,
	OUT_Undefined,
	OUT_02_CONIN_Flush,
	OUT_03_CONIN_SetIntLevel,
	OUT_Undefined,
	OUT_05_CONOUT,
	OUT_Undefined,
	OUT_07_CONOUT_Flush,
	OUT_08_CONOUT_SetIntLevel,
	OUT_Undefined,
	OUT_0A_DSK_SelectDisk,
	OUT_0B_DSK_WritePos_Low,
	OUT_0C_DSK_WritePos_Mid,
	OUT_0D_DSK_WritePos_High,
	OUT_0E_DSK_WriteBuf_Low,
	OUT_0F_DSK_WriteBuf_High,
	OUT_10_DSK_WriteLen_Low,
	OUT_11_DSK_WriteLen_High,
	OUT_12_DSK_Write,
	OUT_13_DSK_WriteIntLevel,
	OUT_14_DSK_ReadPos_Low,
	OUT_15_DSK_ReadPos_Mid,
	OUT_16_DSK_ReadPos_High,
	OUT_17_DSK_ReadBuf_Low,
	OUT_18_DSK_ReadBuf_High,
	OUT_19_DSK_ReadLen_Low,
	OUT_1A_DSK_ReadLen_High,
	OUT_1B_DSK_Read,
	OUT_1C_DSK_ReadIntLevel,
	OUT_1D_DEBUG_IntLevel,
	OUT_1E_DEBUG_BreakPointAddress,
	OUT_1F_LED_Set
};
