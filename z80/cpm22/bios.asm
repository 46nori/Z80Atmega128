;
;       CP/M 2.2 BIOS for Z80ATmega128
;
MEM	        .equ    62              ; 62K CP/M
CCP_ENTRY       .equ    (MEM-7)*1024
BDOS_ENTRY      .equ    CCP_ENTRY+0x800
BIOS_ENTRY      .equ    CCP_ENTRY+0x1600
VECT_TABLE      .equ    (BIOS_ENTRY + 0x100) & 0xff00
IOBYTE          .equ    0x0003

; Emulated I/O address 
PORT_CONIN      .equ    0x00
PORT_CONIN_STS  .equ    0x01
PORT_CONIN_INT  .equ    0x03
PORT_CONOUT     .equ    0x05
PORT_CONOUT_STS .equ    0x06
PORT_CONOUT_INT .equ    0x08
PORT_SELDSK     .equ    0x0A
PORT_LED        .equ    0x1F

        .area   BIOS (ABS)
        .org    BIOS_ENTRY

;******************************************************************
;   BIOS jump table
;******************************************************************
_CBOOT:     JP CBOOT
_WBOOT:     JP WBOOT
_CONST:     JP CONST
_CONIN:     JP CONIN
_CONOUT:    JP CONOUT
_LIST:      JP LIST
_PUNCH:     JP PUNCH
_READER:    JP READER
_HOME:      JP HOME
_SELDSK:    JP SELDSK
_SETTRK:    JP SETTRK
_SETSEC:    JP SETSEC
_SETDMA:    JP SETDMA
_READ:      JP READ
_WRITE:     JP WRITE
_PRSTAT:    JP PRSTAT
_SECTRN:    JP SECTRN

;******************************************************************
;   Interrupt vector table
;******************************************************************
        .org VECT_TABLE
        .dw     ISR_00          ; INT 0
        .dw     ISR_01          ; INT 1

;******************************************************************
;   Interrupt handler
;******************************************************************
ISR_00:
        EXX
        ; debug
        LD A, 2
        CALL LED_ON
        EXX
        EI
        RETI

ISR_01:
        EXX
        ; debug 
        LD A, 2
        CALL LED_ON
        EXX
        EI
        RETI

;******************************************************************
;   Initialization
;******************************************************************
;   Load CCP and BDOS
LOAD_CCP_BDOS:
        ; debug
        LD HL, MSG_CCP_BDOS
        CALL PRINT_STR
        RET
MSG_CCP_BDOS:
        .str    "Loaded CCP & BDOS.\r\n"
        .db     0

;******************************************************************
;   00: Cold Boot
;******************************************************************
CBOOT:
        DI
        LD SP, CCP_ENTRY
        LD HL, VECT_TABLE
        LD A, H
        LD i, A
        IM 2

        ; Disable interrupt of emulated Console device
        LD A, 128
        OUT (PORT_CONIN_INT), A
        OUT (PORT_CONOUT_INT), A
        EI

;        LD A, 7
;        CALL LED_ON

        ; Boot message
        LD HL, BOOT_MSG
        CALL PRINT_STR

        ; Set IOBYTE
        LD A, 0
        LD (IOBYTE), A

        JP CCP_ENTRY

BOOT_MSG:
        .str    "\r\n"
        .str    "CP/M-80 Ver2.2 on Z80ATmega128\r\n"
        .db     0

;******************************************************************
;   01: Warm Boot
;******************************************************************
WBOOT:
        ; Reload CCP and BDOS
        CALL LOAD_CCP_BDOS

        ; Set 'JP _WBOOT' at 0x0000
        LD A, 0xC3
        LD (0x0000), A
        LD HL, _WBOOT
        LD (0x0001), HL

        ; Set 'JP BDOS_ENTRY + 6' at 0x0005
        LD A, 0xC3
        LD (0x0005), A
        LD HL, BDOS_ENTRY + 6 
        LD (0x0006), HL

        ; Set default drive as A:(0)
        LD C, 0

        LD SP, CCP_ENTRY
        JP CCP_ENTRY + 3

;******************************************************************
;   02: Get status of CON:
;******************************************************************
CONST:
        IN A, (PORT_CONIN_STS)
        OR A
        RET Z
        LD A, 0xFF
        RET

;******************************************************************
;   03: Input a character from CON:
;******************************************************************
CONIN:
        IN A, (PORT_CONIN)
        RET

;******************************************************************
;   04: Output a character to CON:
;******************************************************************
CONOUT:
        PUSH AF
        LD A, C
        OUT (PORT_CONOUT), A
WAIT_CONOUT:
        IN A, (PORT_CONOUT_STS)
        OR A
        JR NZ, WAIT_CONOUT
        POP AF
        RET

;******************************************************************
;   05: Output a character to LST: (Not implemented)
;******************************************************************
LIST:
        RET

;******************************************************************
;   06: Output a character to PUN: (Not implemented)
;******************************************************************
PUNCH:
        RET

;******************************************************************
;   07: Input a character from RDR: (Not implemented)
;******************************************************************
READER:
        LD A, 0x1A      ; Terminator
        RET

;******************************************************************
;   08: Return selected DISK to home position
;******************************************************************
HOME:
        RET

;******************************************************************
;   09: Select DISK
;******************************************************************
SELDSK:
        LD A, C
        OUT (PORT_SELDSK), A
        LD (DRIVE_NO), A
;        LD HL, DPH00
        LD HL, 0
        RET
DRIVE_NO:
        .db     0

;******************************************************************
;   10: Set logical track number
;******************************************************************
SETTRK:
        RET

;******************************************************************
;   11: Set logical sector number
;******************************************************************
SETSEC:
        RET

;******************************************************************
;   12: Set data buffer address
;******************************************************************
SETDMA:
        RET

;******************************************************************
;   13: Read a record from DISK
;******************************************************************
READ:
        LD A, 1         ; Hardware error
        RET

;******************************************************************
;   14: Write a record to DISK
;******************************************************************
WRITE:
        LD A, 1         ; Hardware error
        RET

;******************************************************************
; 15: Check status of LST: (Not implemented)
;******************************************************************
PRSTAT:
        XOR A           ; Not ready
        RET

;******************************************************************
;   16: Translate logical sector to physical sector
;******************************************************************
SECTRN:
        ; if DE == 0 then return with HL=DE
        LD H, D
        LD L, E
        LD A, D
        OR E
        RET Z

        ; Translation
        RET

;******************************************************************
;    Utilities
;******************************************************************;
;================================================
; Print string
;  HL : address of the string terminated by 0x00
;  A,C are used internally
;================================================
PRINT_STR:
        LD A, (HL)
        OR A
        RET Z
        LD C, A
        CALL CONOUT
        INC HL
        JR PRINT_STR

;================================================
; LED ON/OFF
;  A : bit 0: BLUE
;      bit 1: YELLOW
;      bit 2: RED
;  C is used internally
;================================================
LED_ON:
        CPL
        LD C, A
        IN A, (PORT_LED)
        AND C
        OUT (PORT_LED), A
        RET
LED_OFF:
        LD C, A
        IN A, (PORT_LED)
        OR C
        OUT (PORT_LED), A
        RET

;******************************************************************
;       Disk Parameter Header (DPH)
;******************************************************************
; DISK A
DPH00:
        .dw    0               ; XLT: Translation Table address (0 if no table)
        .dw    0               ; SPA: scratch pad area 1
        .dw    0               ; SPA: scratch pad area 2
        .dw    0               ; SPA: scratch pad area 3
        .dw    DIRB00          ; DIRB: Directory Buffer address
        .dw    DPB00           ; DPB: Disk Parameter Block address
        .dw    CSV00           ; CSV: Check sum vector address
        .dw    ALV00           ; ALV: Allocation(bit map) vector address
;
;       Disk Parameter Block (DPB)
;
DPB00:  .dw    16              ;sectors per track from bios.
        .db    2               ;block shift.sector in a block 128*2^n
        .db    3               ;block mask.sector no. in a block - 1
        .db    0               ;extent mask.
        .dw    23              ;disk size (number of blocks-1).
        .dw    31              ;directory size.(max file name no.-1)
        .dw    0xC0            ;storage for first bytes of bit map (dir space used).
        .dw    8               ;check sum vector size
        .dw    0               ;offset. first usable track number.
;
DIRB00: .ds     128
CSV00:  .db     0,0,0,0,0,0,0,0
ALV00:  .db     0,0,0

;******************************************************************
;   Disk Parameter Block (DPB)
;******************************************************************
DPB:
SECTORS:.dw	0		;sectors per track from bios.
BLKSHFT:.db	0		;block shift.
BLKMASK:.db	0		;block mask.
EXTMASK:.db	0		;extent mask.
DSKSIZE:.dw	0		;disk size from bios (number of blocks-1).
DIRSIZE:.dw	0		;directory size.
ALLOC0:	.dw	0		;storage for first bytes of bit map (dir space used).
ALLOC1:	.dw	0
OFFSET:	.dw	0		;first usable track number.
XLATE:	.dw	0		;sector translation table address.

;******************************************************************
;   DMA buffer
;******************************************************************
DMABUF:
        .ds     512
