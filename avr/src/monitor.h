/*
 * monitor.h
 *
 * Created: 2023/03/31 20:21:52
 *  Author: 46nori
 */ 


#ifndef MONITOR_H_
#define MONITOR_H_

extern void monitor(void);
extern int load_eeprom_extmem(uint8_t *dst, const uint8_t *src, size_t size);

#endif /* MONITOR_H_ */