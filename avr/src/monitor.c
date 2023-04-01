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

static int r_xmodem(uint8_t *dest, size_t *size);
static int s_xmodem(uint8_t *src, size_t size);

void monitor()
{
	
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

#define TOUT    (uint32_t)1000000		// 1 sec
#define TOUT0   (uint32_t)10000000		// 10 sec
#define PKT_SIZE  128
#define MAX_RETRY 10

static int copy(uint8_t *dst, uint8_t *src, size_t size) {
	memcpy(dst, src, size);
	return 0;
}

/**
 *    @brief XMODEM(128-SUM) receive
 *
 *    @param [in]  dest : destination memory address
 *           [out] size : received data bytes
 *    @retval  0  Success
 *    @retval -1  Transfer failed
 *    @retval -2  Timed out
 */
static int r_xmodem(uint8_t *dest, size_t *size)
{
    int i, retry;
    int seq, sum, c, c2;
    size_t blk, count;
    uint8_t buf[128];

    *size   = 0;
    count   = 0;
    blk     = 1;
    retry   = 0;
    while (1) {
        switch (c = x_getchar_tout(TOUT0)) {
        case SOH:
            seq   = 0xff & blk;
            c     = x_getchar_tout(TOUT);			// Get <blk>
            c2    = x_getchar_tout(TOUT);			// Get <~blk>
            if ((c == seq) && (c2 == (0xff & ~seq))) {
                sum = 0;							// Header was correct
                for (i = 0; i < PKT_SIZE; i++) {
                    if ((c = x_getchar_tout(TOUT)) == EOF) {
                        x_putchar(NAK);				// Resending request due to time out
                        goto skip_this_block;
                    }
                    buf[i] = c;
                    sum += c ;
                }
                if ((sum & 0xff) == x_getchar_tout(TOUT0)) {	// Get <sum>
                    if ((c = copy(dest, buf, 128)) != 0) {
                        x_putchar(CAN);
                        return c;
                    }
                    x_putchar(ACK);					// <sum> was correct
                    retry = 0;
                    blk++;
                    count++;
                    dest += PKT_SIZE;
                } else {							// <sum> was incorrect
                    if (++retry > MAX_RETRY) {
                        x_putchar(CAN);				// Give up
                        return -1;
                    }
                    x_putchar(NAK);					// Retry to receive
                }
            } else {								// Header was incorrect
                retry++;
                for (i = 0; i < PKT_SIZE+1; i++) {	// Skip this block
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
                if (count > 0)
                    return -1;
                else
                    return -2;		// Timed out
            }
            x_putchar(NAK);
            break;

        }
    }
}

/**
 *    @brief XMODEM(128-CRC/SUM) send
 *
 *    @param [in] src  : source address
 *           [in] size : transfer bytes
 *    @retval  0  Success
 *    @retval -1  Transfer failed
 *    @retval -2  Aborted by receiver
 */
static int s_xmodem(uint8_t *src, size_t size)
{
    int i, j, retry;
    int seq, sum, c, d, crc_flag, complete;
    size_t blk, count;
    unsigned short crc;

    retry = 0;
    count = 0;
    blk   = 1;
    crc_flag = 0;
    complete = 0;
    do {

        switch (c = x_getchar_tout(TOUT0)) {

        case CCC:					// CRC mode
            crc_flag = 1;
            goto SEND;

        case NAK:
            if (count > 0) {		// Retry to send a packet
                --count;
                --blk;
                src -= PKT_SIZE;
            } else {
                crc_flag = 0;		// Check-sum mode
            }

        SEND:
        case ACK:
            if (count < size) {	//  Send a packet
                seq = 0xff & blk;
                x_putchar(SOH);
                x_putchar(seq);
                x_putchar(~seq);
                crc = sum = 0;
                for (i = 0; i < PKT_SIZE; i++) {
                    d = *src++;
                    x_putchar((char)d);
                    if (crc_flag) {
                        crc = crc ^ (unsigned short)d << 8;
                        for (j = 0; j < 8; ++j) {
                            if (crc & 0x8000)
                                crc = crc << 1 ^ 0x1021;
                            else
                                crc = crc << 1;
                        }
                    } else {
                        sum += d;
                    }
                }
                if (crc_flag) {
                    x_putchar((crc >> 8) & 0xff);
                    x_putchar( crc       & 0xff);
                } else {
                    x_putchar(sum & 0xff);
                }
                ++count;
                ++blk;
                retry = 0;
            } else {				// Transfer complete
                complete = 1;
            }
            break;

        case CAN:
            return -2;				// Aborted by receiver

        case EOF:					// timed out
            if (count > 0) {
                if (++retry > MAX_RETRY)
                    return -1;		// Give up
            }
            break;

        default:
            break;

        }

    } while (!complete);

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
