;
;       CP/M 2.2 BIOS for Z80ATmega128
;
MEM	    .equ        62      ; 62k CP/M
CCP_ENTRY   .equ        0x3400+(MEM-20)*1024
BDOS_ENTRY  .equ        CCP_ENTRY+0x800
BIOS_ENTRY  .equ        CCP_ENTRY+0x1600
VECT_TABLE  .equ        (BIOS_ENTRY + 0x100) & 0xff00

; Emulated I/O address 
PORT_CONIN      .equ        0x00
PORT_CONIN_STS  .equ        0x01
PORT_CONOUT     .equ        0x05
PORT_CONOUT_STS .equ        0x06
PORT_SELDSK     .equ        0x0A

        .area   BIOS (ABS)
        .org    BIOS_ENTRY

;******************************************************************
;   BIOS jump table
;******************************************************************
_BOOT:      JP BOOT
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
;   Disk Parameter Header (DPH)
;******************************************************************
DPH:
        .dw     XLATE

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
        LD C, "#"
        CALL CONOUT
        EXX
        EI
        RETI

ISR_01:
        EXX
        EXX
        EI
        RETI

;******************************************************************
;   DMA buffer
;******************************************************************
DMABUF_RD:
        .ds     512
DMABUF_WR:
        .ds     512

;******************************************************************
;   Initialization
;******************************************************************
;   Load CCP and BDOS
LOAD_CCP_BDOS:
        ; debug
        LD B, 20
        LD C, PORT_CONOUT
        LD HL, MSG_CCP_BDOS
        OTIR
        RET
MSG_CCP_BDOS:
        .str    "Loaded CCP & BDOS.\r\n"

;   Load BIOS
LOAD_BIOS:
        ; debug
        LD B, 14
        LD C, PORT_CONOUT
        LD HL, MSG_BIOS
        OTIR
        RET
MSG_BIOS:
        .str    "Loaded BIOS.\r\n"

;******************************************************************
;   00: Cold Boot
;******************************************************************
BOOT:
        DI
        LD SP, CCP_ENTRY
        LD HL, VECT_TABLE
        LD A, H
        LD i, A
        IM 2
        EI
    
        CALL LOAD_BIOS
        CALL LOAD_CCP_BDOS

        JP CCP_ENTRY

;******************************************************************
;   01: Warm Boot
;******************************************************************
WBOOT:
        ; Reload CCP and BDOS
        CALL LOAD_CCP_BDOS

        PUSH HL
        ; Set 'JP _WBOOT' at 0x0000
        LD A, 0xC3
        LD (0x0000), A
        LD HL, _WBOOT
        LD (0x0001), HL

        ; Set 'JP BDOS_ENTRY' at 0x0005
        LD A, 0xC3
        LD (0x0005), A
        LD HL, BDOS_ENTRY
        LD (0x0006), HL
        POP HL

        ; Set default drive 0
        XOR A
        LD C, A

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
        LD HL, 0        ; no disk
        RET

;******************************************************************
;   10: Set logical track number
;******************************************************************
SETTRK:
        LD HL, DPH
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
