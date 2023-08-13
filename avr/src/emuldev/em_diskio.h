/*
 * em_diskio.h
 *
 * Created: 2023/06/28 22:30:52
 *  Author: 46nori
 */ 

#ifndef EM_DISKIO_H_
#define EM_DISKIO_H_
#include "asf.h"

#define TEMPLATE_IN_OUT_EXTERN(INTNUM,FUNC) \
extern uint8_t IN_##INTNUM##_##FUNC(void);\
extern void OUT_##INTNUM##_##FUNC(uint8_t data);

extern void init_em_diskio(void);
extern void em_disk_read(void);
extern void em_disk_write(void);
extern uint8_t	IN_0A_DSK_GetDiskStatus(void);
extern void		OUT_0A_DSK_SelectDisk(uint8_t data);
TEMPLATE_IN_OUT_EXTERN(0B,DSK_WritePos)
TEMPLATE_IN_OUT_EXTERN(0C,DSK_WriteBuf)
TEMPLATE_IN_OUT_EXTERN(0D,DSK_WriteLen)
extern uint8_t	IN_0E_DSK_WriteStatus(void);
extern void		OUT_0E_DSK_Write(uint8_t data);
extern uint8_t	IN_0F_DSK_WriteIntlevel(void);
extern void		OUT_0F_DSK_WriteIntLevel(uint8_t data);
TEMPLATE_IN_OUT_EXTERN(10,DSK_ReadPos)
TEMPLATE_IN_OUT_EXTERN(11,DSK_ReadBuf)
TEMPLATE_IN_OUT_EXTERN(12,DSK_ReadLen)
extern uint8_t	IN_13_DSK_ReadStatus(void);
extern void		OUT_13_DSK_Read(uint8_t data);
extern uint8_t	IN_14_DSK_ReadIntLevel(void);
extern void		OUT_14_DSK_ReadIntLevel(uint8_t data);

#endif /* EM_DISKIO_H_ */