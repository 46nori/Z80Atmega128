/*
 * xmodem.c
 *
 * Created: 2023/04/02 15:16:45
 *  Author: 46nori
 */ 
#include <stdio.h>
#include <string.h>
#include "xmodem.h"
#include "xconsoleio.h"

//
// Constant of time out value.
// Should be configured according to the platform.
//
#define TOUT	(uint32_t)400000	// 1 sec (us unit)
#define TOUT0	(uint32_t)3340000	// 10 sec

//
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
#define PKT_SIZE	128
#define MAX_RETRY	9
#define SOH			0x01
#define EOT			0x04
#define ACK			0x06
#define NAK			0x15
#define CAN			0x18
#define CCC			0x43

/**
 *    @brief copy receive data.
 *           User can override this function.
 *
 *    @param [in] dst  : destination address
 *           [in] src  : source buffer address
 *           [in] size : copy bytes
 *    @retval  0  Success
 *    @retval -4  copy error
 */
static int copy(unsigned char *dst, unsigned char *src, size_t size) {
	memcpy(dst, src, size);
	return 0;
}

/**
 *    @brief XMODEM(128-SUM) receiver
 *
 *    @param [in]  dst   : destination memory address
 *           [out] size  : received data bytes
 *           [in]  cfunc : copy function to write a XMODEM block to dst
 *    @retval  0  Success
 *    @retval -1  Transfer failed
 *    @retval -2  Timed out
 *    @retval -4  copy error
 */
int r_xmodem(unsigned char *dst, size_t *size, copyfunc cfunc)
{
    int seq, sum, c, c2;
    unsigned char buf[128];

	if (cfunc == NULL) {
		cfunc = copy;
	}

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
                    if ((c = (*cfunc)(dst, buf, 128)) != 0) {
                        x_putchar(CAN);	// Give up due to copy error
                        return -1;
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
 *    @brief XMODEM(128-CRC/SUM) sender
 *
 *    @param [in] src    : source address
 *           [in] blocks : transfer blocks
 *    @retval  0  Success
 *    @retval -1  Transfer failed
 *    @retval -3  Aborted by receiver
 */
int s_xmodem(unsigned char *src, size_t blocks)
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
                        crc = crc ^ (unsigned int)d << 8;
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
