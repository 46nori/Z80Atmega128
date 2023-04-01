/*
 * monitor.c
 *
 * Created: 2023/03/31 20:22:13
 *  Author: 46nori
 */ 
#include <asf.h>
#include <string.h>
#include "monitor.h"
#include "usart.h"

static int r_xmodem(uint8_t *dst, size_t *size);
static int s_xmodem(uint8_t *src, size_t blocks);


void monitor(void)
{
#if 0
	// measure accuracy of timeout parameter 
	int c;
	while (1) {
		c = x_getchar_tout(400000);
		if (c == EOF) {
			x_putchar('/');
			continue;		
		}
		x_putchar(c);
	}
#endif
	
#if 0
	// s_xmodem() test
	static uint8_t test[1024];
	for (int i = 0; i < 1024; i++) {
		test[i] = i;
	}
	int st = s_xmodem(test, 1024/128);
	switch (st) {
		case -1:
			x_puts("Transfer error");
			break;
		case -2:
			x_puts("Timed out.");
			break;
		case -3:
			x_puts("Aborted by receiver.");
			break;
		default:
			x_puts("Success.");
			break;
	}
	while(1);
#endif

#if 1
	// r_xmodem() test
	size_t size;
	static uint8_t test[1024];
	int st = r_xmodem(test, &size);
	switch (st) {
		case -1:
		x_puts("Transfer error");
		break;
		case -2:
		x_puts("Timed out.");
		break;
		case -4:
		x_puts("Copy error.");
		break;
		default:
		x_puts("Success.");
		break;
	}
	if (size == 1024) {
		x_puts("size is 1024.");
	}
	while(1);
#endif
	
}

//    XMODEM data transfer protocol (128byte Check-sum)
//
//    Packet format: <SOH> <blk> <~blk> <128byte data> <sum>
//
//    <SOH>  : 0x01
//    <blk>  : Block count, which starts from 0x01
//             and wraps 0xff to 0x00 (not to 0x01).
//    <~blk> : 1 complemented of <blk>.
//    <sum>  : the sum of data bytes only to use check-sum.
//
#define SOH     0x01
#define EOT     0x04
#define ACK     0x06
#define NAK     0x15
#define CAN     0x18
#define CCC     0x43

#define TOUT    (uint32_t)400000		// 1 sec
#define TOUT0   (uint32_t)4000000		// 10 sec
#define PKT_SIZE  128
#define MAX_RETRY 20

/**
 *    @brief copy receive data 
 *
 *    @param [in] dst  : destination address
 *           [in] src  : source buffer address
 *           [in] size : copy bytes
 *    @retval  0  Success
 *    @retval -4  copy error
 */
static int copy(uint8_t *dst, uint8_t *src, size_t size) {
	memcpy(dst, src, size);
	return 0;
}

/**
 *    @brief XMODEM(128-SUM) receive
 *
 *    @param [in]  dst : destination memory address
 *           [out] size : received data bytes
 *    @retval  0  Success
 *    @retval -1  Transfer failed
 *    @retval -2  Timed out
 *    @retval -4  copy error
 */
static int r_xmodem(uint8_t *dst, size_t *size)
{
    int seq, sum, c, c2;
    uint8_t buf[128];

    size_t count = 0;
    int retry = 0;
    *size = 0;
    while (1) {
        switch (c = x_getchar_tout(TOUT0)) {
        case SOH:
            seq = 0xff & (count + 1);
            c   = x_getchar_tout(TOUT);	// Get <blk>
            c2  = x_getchar_tout(TOUT);	// Get <~blk>
            if ((c == seq) && (c2 == (0xff & ~seq))) {
				// Header was correct
                sum = 0;
                for (int i = 0; i < PKT_SIZE; i++) {
                    if ((c = x_getchar_tout(TOUT)) == EOF) {
                        x_putchar(NAK);	// Retry due to time out
                        goto skip_this_block;
                    }
                    buf[i] = c;
                    sum += c ;
                }
				// Check <sum>
                if ((sum & 0xff) == x_getchar_tout(TOUT0)) {
                    if ((c = copy(dst, buf, 128)) != 0) {
                        x_putchar(CAN);
                        return c;
                    }
                    x_putchar(ACK);	// <sum> was correct
		            *size = count * PKT_SIZE;
                    retry = 0;
                    count++;
                    dst += PKT_SIZE;
                } else {
					// <sum> was incorrect
                    if (++retry > MAX_RETRY) {
                        x_putchar(CAN);	// Give up
                        return -1;
                    }
                    x_putchar(NAK);	// Retry due to check sum error
                }
            } else {
				// Header was incorrect
                retry++;
                for (int i = 0; i < PKT_SIZE+1; i++) {
					// Skip this block
                    if ((c = x_getchar_tout(TOUT0)) == EOF) {
                        if (++retry > MAX_RETRY) {
                            x_putchar(CAN);
                            return -1;
                        } else {
                            break;
                        }
                    }
                }
                x_putchar(NAK);
            }
            skip_this_block:;
            break;

        case EOT:
            x_putchar(ACK);
            *size = count * PKT_SIZE;
            return 0;
            break;

        case CAN:
            return -1;
            break;

        case EOF:
            if (++retry > MAX_RETRY) {
                x_putchar(CAN);
                if (count > 0) {
                    return -1;		// Transfer failed
                } else {
                    return -2;		// Timed out
				}
            }
            x_putchar(NAK);
            break;
        }
    }
}

/**
 *    @brief XMODEM(128-CRC/SUM) send
 *
 *    @param [in] src    : source address
 *           [in] blocks : transfer blocks
 *    @retval  0  Success
 *    @retval -1  Transfer failed
 *    @retval -3  Aborted by receiver
 */
static int s_xmodem(uint8_t *src, size_t blocks)
{
    int seq, sum, crc, c, d;

    size_t count = 0;
    int retry = 0;
    int is_crc = 0;
    int is_end = 0;
    do {
        switch (c = x_getchar_tout(TOUT0)) {
        case CCC:					// CRC mode
            is_crc = 1;
            goto SEND;

        case NAK:
            if (count > 0) {		// Retry to send a packet
                --count;
                src -= PKT_SIZE;
            } else {
                is_crc = 0;		// Check-sum mode
            }

        SEND:
        case ACK:
            if (count < blocks) {	//  Send a packet
                seq = 0xff & (count + 1);
                x_putchar(SOH);
                x_putchar(seq);
                x_putchar(~seq);
                crc = sum = 0;
                for (int i = 0; i < PKT_SIZE; i++) {
                    d = *src++;
                    x_putchar(d);
                    if (is_crc) {
                        crc = crc ^ (uint16_t)d << 8;
                        for (int j = 0; j < 8; ++j) {
                            if (crc & 0x8000)
                                crc = crc << 1 ^ 0x1021;
                            else
                                crc = crc << 1;
                        }
                    } else {
                        sum += d;
                    }
                }
                if (is_crc) {
                    x_putchar((crc >> 8) & 0xff);
                    x_putchar( crc       & 0xff);
                } else {
                    x_putchar(sum & 0xff);
                }
                ++count;
                retry = 0;
            } else {				// Transfer is_end
                is_end = 1;
            }
            break;

        case CAN:
            return -3;				// Aborted by receiver

        case EOF:					// timed out
            if (count > 0) {
                if (++retry > MAX_RETRY)
                    return -1;		// Give up
            }
            break;

        default:
            break;
        }
    } while (!is_end);

    retry = 0;
    do {
        x_putchar(EOT);				// Send EOT and wait for ACK
        c = x_getchar_tout(TOUT0);
        if (c == EOF) {
            if (++retry > MAX_RETRY)
                return -1;			// Give up
        }
    } while (c != ACK);

    return 0;
}
