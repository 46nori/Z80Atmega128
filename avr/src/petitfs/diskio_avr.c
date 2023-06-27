/*-------------------------------------------------------------------------*/
/* PFF - Low level disk control module for AVR							   */
/* Modified from (C)ChaN, 2014											   */
/*-------------------------------------------------------------------------*/
#define F_CPU 16000000UL		// Workaround

#include "pff.h"
#include "diskio.h"

/*-------------------------------------------------------------------------*/
/* Platform dependent macros and functions MODIFY FOR YOUR DEVICE           */
/*-------------------------------------------------------------------------*/

#include <avr/io.h> /* Device specific include files */

#define SPIPORT  PORTB
#define SPI_CS   _BV(PORTB0) //(1 << 0)  /* PB0 */
#define SPI_SCK  _BV(PORTB1) //(1 << 1)  /* PB1 */
#define SPI_MOSI _BV(PORTB2) //(1 << 2)  /* PB2 SD card DI */
#define SPI_MISO _BV(PORTB3) //(1 << 3)  /* PB3 SD card DO */

/* Port controls  (Platform dependent) */
#define SELECT()   SPIPORT &= ~SPI_CS /* CS = L */
#define DESELECT() SPIPORT |= SPI_CS  /* CS = H */
#define SELECTING ((DDRB & SPI_CS) && !(PORTB & SPI_CS))

static void init_spi(void)
{
	/* SPI pins */
	DDRB  |= (SPI_MOSI | SPI_CS | SPI_SCK); /* Set outputs */
	PORTB |= SPI_MISO; /* Enable pull-up on SPI_MISO (SD card DO) */

	SPCR = _BV(SPE)  |	// Enable SPI
	       _BV(MSTR) |	// Master mode
		   _BV(SPR1);	// SPI prescaler to give 100-400kHz clock
						// 1/64 With 16MHz F_CPU
}

static uint8_t spi(uint8_t d)
{
	SPDR = d;
	while (!(SPSR & _BV(SPIF)))
	;
	return SPDR;
}

static void xmit_spi(uint8_t d)
{
	spi(d);
}

static uint8_t rcv_spi(void)
{
	return spi(0xFF);
}

/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

/* Definitions for MMC/SDC command */
#define CMD0 (0x40 + 0)    /* GO_IDLE_STATE */
#define CMD1 (0x40 + 1)    /* SEND_OP_COND (MMC) */
#define ACMD41 (0xC0 + 41) /* SEND_OP_COND (SDC) */
#define CMD8 (0x40 + 8)    /* SEND_IF_COND */
#define CMD16 (0x40 + 16)  /* SET_BLOCKLEN */
#define CMD17 (0x40 + 17)  /* READ_SINGLE_BLOCK */
#define CMD24 (0x40 + 24)  /* WRITE_BLOCK */
#define CMD55 (0x40 + 55)  /* APP_CMD */
#define CMD58 (0x40 + 58)  /* READ_OCR */

/* Card type flags (CardType) */
#define CT_MMC 0x01   /* MMC ver 3 */
#define CT_SD1 0x02   /* SD ver 1 */
#define CT_SD2 0x04   /* SD ver 2 */
#define CT_BLOCK 0x08 /* Block addressing */

BYTE CardType = 0;

/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/
static BYTE send_cmd(BYTE  cmd, /* 1st byte (Start + Index) */
                     DWORD arg  /* Argument (32 bits) */
)
{
	BYTE n, res;

	if (cmd & 0x80) { /* ACMD<n> is the command sequence of CMD55-CMD<n> */
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);
		if (res > 1)
			return res;
	}

	/* Select the card */
	DESELECT();
	rcv_spi();
	SELECT();
	rcv_spi();

	/* Send a command packet */
	xmit_spi(cmd);               /* Start + Command index */
	xmit_spi((BYTE)(arg >> 24)); /* Argument[31..24] */
	xmit_spi((BYTE)(arg >> 16)); /* Argument[23..16] */
	xmit_spi((BYTE)(arg >> 8));  /* Argument[15..8] */
	xmit_spi((BYTE)arg);         /* Argument[7..0] */
	n = 0x01;                    /* Dummy CRC + Stop */
	if (cmd == CMD0)
		n = 0x95; /* Valid CRC for CMD0(0) */
	if (cmd == CMD8)
		n = 0x87; /* Valid CRC for CMD8(0x1AA) */
	xmit_spi(n);

	/* Receive a command response */
	n = 10; /* Wait for a valid response in timeout of 10 attempts */
	do {
		res = rcv_spi();
	} while ((res & 0x80) && --n);

	return res; /* Return with the response value */
}

/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(void)
{
	BYTE n, cmd, ty, ocr[4];
	UINT tmr;

#if _USE_WRITE
	if (CardType && SELECTING)
		disk_writep(0, 0); /* Finalize write process if it is in progress */
#endif

	init_spi(); /* Initialize ports to control MMC */
	DESELECT();
	for (n = 10; n; n--)
		rcv_spi(); /* 80 dummy clocks with CS=H */

	ty = 0;
	if (send_cmd(CMD0, 0) == 1) {         /* GO_IDLE_STATE */
		if (send_cmd(CMD8, 0x1AA) == 1) { /* SDv2 */
			for (n = 0; n < 4; n++)
				ocr[n] = rcv_spi();                 /* Get trailing return value of R7 resp */
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) { /* The card can work at vdd range of 2.7-3.6V */
				for (tmr = 10000; tmr && send_cmd(ACMD41, 1UL << 30); tmr--)
					_delay_us(100);                   /* Wait for leaving idle state (ACMD41 with HCS bit) */
				if (tmr && send_cmd(CMD58, 0) == 0) { /* Check CCS bit in the OCR */
					for (n = 0; n < 4; n++)
						ocr[n] = rcv_spi();
					ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2; /* SDv2 (HC or SC) */
				}
			}
		} else { /* SDv1 or MMCv3 */
			if (send_cmd(ACMD41, 0) <= 1) {
				ty  = CT_SD1;
				cmd = ACMD41; /* SDv1 */
			} else {
				ty  = CT_MMC;
				cmd = CMD1; /* MMCv3 */
			}
			for (tmr = 10000; tmr && send_cmd(cmd, 0); tmr--)
				_delay_us(100);                    /* Wait for leaving idle state */
			if (!tmr || send_cmd(CMD16, 512) != 0) /* Set R/W block length to 512 */
				ty = 0;
		}
	}
	CardType = ty;
	DESELECT();
	rcv_spi();

	return ty ? 0 : STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Read partial sector                                                   */
/*-----------------------------------------------------------------------*/

DRESULT disk_readp(BYTE *buff,   /* Pointer to the read buffer (NULL:Forward to the stream) */
                   DWORD sector, /* Sector number (LBA) */
                   UINT  offset, /* Byte offset to read from (0..511) */
                   UINT  count   /* Number of bytes to read (ofs + cnt mus be <= 512) */
)
{
	DRESULT res;
	BYTE    rc;
	UINT    bc;

	if (!(CardType & CT_BLOCK))
		sector *= 512; /* Convert to byte address if needed */

	res = RES_ERROR;
	if (send_cmd(CMD17, sector) == 0) { /* READ_SINGLE_BLOCK */

		// bc = 40000;	/* Time counter */
		do { /* Wait for response */
			rc = rcv_spi();
		} while (rc == 0xFF);

		if (rc == 0xFE) { /* A data packet arrived */

			bc = 512 + 2 - offset - count; /* Number of trailing bytes to skip */

			/* Skip leading bytes */
			while (offset--)
				rcv_spi();

			/* Receive a part of the sector */
			if (buff) { /* Store data to the memory */
				do {
					*buff++ = rcv_spi();
				} while (--count);
			} else { /* Forward data to the outgoing stream */
				do {
					// FORWARD(rcv_spi());
				} while (--count);
			}

			/* Skip trailing bytes and CRC */
			do
				rcv_spi();
			while (--bc);

			res = RES_OK;
		}
	}

	DESELECT();
	rcv_spi();

	return res;
}

	/*-----------------------------------------------------------------------*/
	/* Write partial sector                                                  */
	/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_writep(const BYTE *buff, /* Pointer to the bytes to be written (NULL:Initiate/Finalize sector write) */
                    DWORD       sc    /* Number of bytes to send, Sector number (LBA) or zero */
)
{
	DRESULT     res;
	UINT        bc;
	static UINT wc; /* Sector write counter */

	res = RES_ERROR;

	if (buff) { /* Send data bytes */
		bc = sc;
		while (bc && wc) { /* Send data bytes to the card */
			xmit_spi(*buff++);
			wc--;
			bc--;
		}
		res = RES_OK;
	} else {
		if (sc) { /* Initiate sector write process */
			if (!(CardType & CT_BLOCK))
				sc *= 512;                  /* Convert to byte address if needed */
			if (send_cmd(CMD24, sc) == 0) { /* WRITE_SINGLE_BLOCK */
				xmit_spi(0xFF);
				xmit_spi(0xFE); /* Data block header */
				wc  = 512;      /* Set byte counter */
				res = RES_OK;
			}
		} else { /* Finalize sector write process */
			bc = wc + 2;
			while (bc--) {
				xmit_spi(0); /* Fill left bytes and CRC with zeros */
			}
			do {
				res = rcv_spi();
			} while (res == 0xFF);
			if ((res & 0x1F) == 0x05) { /* Receive data resp and wait for end of write process in timeout of 500ms */
				for (bc = 5000; rcv_spi() != 0xFF && bc; bc--) /* Wait for ready */
					_delay_us(100);
				if (bc)
					res = RES_OK;
			}
			DESELECT();
			rcv_spi();
		}
	}

	return res;
}
#endif
