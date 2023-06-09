/*
 * em_diskio.h
 *
 * Created: 2023/06/28 22:30:52
 *  Author: 46nori
 */ 

#ifndef INCFILE1_H_
#define INCFILE1_H_
#include "asf.h"

extern void init_em_diskio(void);

extern void OUT_0A_DSK_SelectDisk(uint8_t data);
extern void OUT_0B_DSK_WritePos_Low(uint8_t data);
extern void OUT_0C_DSK_WritePos_Mid(uint8_t data);
extern void OUT_0D_DSK_WritePos_High(uint8_t data);
extern void OUT_0E_DSK_WriteBuf_Low(uint8_t data);
extern void OUT_0F_DSK_WriteBuf_High(uint8_t data);
extern void OUT_10_DSK_WriteLen_Low(uint8_t data);
extern void OUT_11_DSK_WriteLen_High(uint8_t data);
extern void OUT_12_DSK_Write(uint8_t data);
extern void OUT_13_DSK_ReadPos_Low(uint8_t data);
extern void OUT_14_DSK_ReadPos_Mid(uint8_t data);
extern void OUT_15_DSK_ReadPos_High(uint8_t data);
extern void OUT_16_DSK_ReadBuf_Low(uint8_t data);
extern void OUT_17_DSK_ReadBuf_High(uint8_t data);
extern void OUT_18_DSK_ReadLen_Low(uint8_t data);
extern void OUT_19_DSK_ReadLen_High(uint8_t data);
extern void OUT_1A_DSK_Read(uint8_t data);
extern uint8_t IN_0A_DSK_GetDiskNumber(void);
extern uint8_t IN_0B_DSK_WritePos_Low(void);
extern uint8_t IN_0C_DSK_WritePos_Mid(void);
extern uint8_t IN_0D_DSK_WritePos_High(void);
extern uint8_t IN_0E_DSK_WriteBuf_Low(void);
extern uint8_t IN_0F_DSK_WriteBuf_High(void);
extern uint8_t IN_10_DSK_WriteLen_Low(void);
extern uint8_t IN_11_DSK_WriteLen_High(void);
extern uint8_t IN_12_DSK_WriteStatus(void);
extern uint8_t IN_13_DSK_ReadPos_Low(void);
extern uint8_t IN_14_DSK_ReadPos_Mid(void);
extern uint8_t IN_15_DSK_ReadPos_High(void);
extern uint8_t IN_16_DSK_ReadBuf_Low(void);
extern uint8_t IN_17_DSK_ReadBuf_High(void);
extern uint8_t IN_18_DSK_ReadLen_Low(void);
extern uint8_t IN_19_DSK_ReadLen_High(void);
extern uint8_t IN_1A_DSK_ReadStatus(void);


#endif /* INCFILE1_H_ */