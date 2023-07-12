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
DISK_WRITEPOS_L .equ    0x0B
DISK_WRITEPOS_M .equ    0x0C
DISK_WRITEPOS_H .equ    0x0D
DISK_WRITEBUF_L .equ    0x0E
DISK_WRITEBUF_H .equ    0x0F
DISK_WRITELEN_L .equ    0x10
DISK_WRITELEN_H .equ    0x11
DISK_WRITE      .equ    0x12
DISK_WRITE_STS  .equ    0x12
DISK_WRITE_INT  .equ    0x13
DISK_READPOS_L  .equ    0x14
DISK_READPOS_M  .equ    0x15
DISK_READPOS_H  .equ    0x16
DISK_READBUF_L  .equ    0x17
DISK_READBUF_H  .equ    0x18
DISK_READLEN_L  .equ    0x19
DISK_READLEN_H  .equ    0x1A
DISK_READ       .equ    0x1B
DISK_READ_STS   .equ    0x1B
DISK_READ_INT   .equ    0x1C
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
        LD HL, DPH00
        RET
DRIVE_NO:
        .db     0

;******************************************************************
;   10: Set logical track number
;******************************************************************
SETTRK:
        PUSH IX
        LD IX, CURRENT_TRACK_NO
        LD (IX+0), C
        LD (IX+1), B
        POP IX
        RET
CURRENT_TRACK_NO:
        .dw     0

;******************************************************************
;   11: Set logical sector number
;******************************************************************
SETSEC:
        PUSH IX
        LD IX, CURRENT_SECTOR_NO
        LD (IX+0), C
        LD (IX+1), B
        POP IX
        RET
CURRENT_SECTOR_NO:
        .dw     0

;******************************************************************
;   12: Set data buffer address
;******************************************************************
SETDMA:
        PUSH IX
        LD IX, DMA_ADRS
        LD (IX+0), C
        LD (IX+1), B
        POP IX
        RET
DMA_ADRS:
        .dw     0

;******************************************************************
;   13: Read a record from DISK
;******************************************************************
READ:
        ; Set destination buffer address
        LD HL, DMABUF
        LD A, H
        OUT (DISK_READBUF_H), A
        LD A, L
        OUT (DISK_READBUF_L), A

        ; Set read length (512byte)
        XOR A
        OUT (DISK_READLEN_H), A
        LD A, 512
        OUT (DISK_READLEN_L), A

        ; A,HL = (TRACK + OFF) * SPT + SECTOR
        LD HL, (CURRENT_SECTOR_NO)
        PUSH HL
        LD HL, (CURRENT_TRACK_NO)
        LD DE, DPB00_OFF
        ADD HL, DE
        LD A, DPB00_SPT
        CALL MUL16_8
        POP DE
        CALL ADD24_16

        ; Check cache
        LD C, A                 ; save A
        LD IX, CACHE_SECTOR_NO
        CP (IX + 0)
        JR NZ, MISHIT_CACHE
        LD A, H
        CP (IX + 1)
        JR NZ, MISHIT_CACHE
        LD A, L
        AND 0xF8
        CP (IX + 2)
        JR NZ, MISHIT_CACHE

        ; Cache hit
        LD A, L
        AND 0x03
        LD B, A                 ; B = L % 4 (blocking factor)
        CALL READ_DMA_BUFFER
        XOR A                   ; Success A=0
        RET

MISHIT_CACHE:
        ; calc 512byte bundary sector
        LD B, L
        LD A, 0xf8
        AND L
        LD L, A                 ; L = L / 4 (truncation)

        LD A, 0x03
        AND B
        LD B, A                 ; B = L % 4 (blocking factor)
        LD A, C                 ; restore A
        ; Here, A,HL = ((TRACK + OFF) * SPT + SECTOR) & 0xfffff8

        ; Set SD Card address to read
        ;  (A,HL) * 128
        ;  aaaaaaaa hhhhhhhh lllllll l000000
        ;             HIGH     MID     LOW
        SRL A
        RRC H
        LD A, H
        OUT (DISK_READPOS_H), A
        RRC L
        LD A, L
        OUT (DISK_READPOS_M), A
        LD A,0
        ADC A 
        OUT (DISK_READPOS_L), A

        ; Read a SD card sector (512bytes)
        OUT (DISK_READ), A
IS_READ_DONE:
        IN A, (DISK_READ_STS)
        OR 1
        JR NZ, IS_READ_DONE
        OR 2
        JR NZ, READ_ERROR

        ; Copy buffer
        CALL READ_DMA_BUFFER

        ; save cache info
        LD IX, CACHE_SECTOR_NO
        LD (IX+0), A
        LD (IX+1), H
        LD (IX+2), L
        LD A, 1
        LD (IS_CACHED), A       ; Set cache flag
        XOR A                   ; Success A=0
        RET

READ_ERROR:
        XOR A
        LD (IS_CACHED), A       ; Clear cache flag
        INC A                   ; Error A=1
        RET

READ_DMA_BUFFER:
        ; BC = 128 * B (blocking factor)
        LD A, B
        SRL A
        LD B, A
        LD C, 0
        ADC C
        LD C, A
        LD HL, DMABUF
        ADD HL, BC              ; HL = source buffer
        LD DE, (DMA_ADRS)
        LD BC, 128
        LDIR
        RET

CACHE_SECTOR_NO:                ; sector # of the 1st DMA buffer
        .db     0, 0, 0
IS_CACHED:
        .db     0

;******************************************************************
;   14: Write a record to DISK
;******************************************************************
WRITE:
        ; Clear cache
        XOR A
        LD (IS_CACHED), A       ; Clear cache flag

        ; Copy write data to DMA buffer
        LD HL, (DMA_ADRS)
        LD DE, DMABUF
        LD BC, 128
        LDIR

        ; Set destination buffer address
        LD HL, DMABUF
        LD A, H
        OUT (DISK_WRITEBUF_H), A
        LD A, L
        OUT (DISK_WRITEBUF_L), A

        ; Set write length (128byte)
        XOR A
        OUT (DISK_WRITELEN_H), A
        LD A, 128
        OUT (DISK_WRITELEN_L), A

        ; A,HL = (TRACK + OFF) * SPT + SECTOR
        LD HL, (CURRENT_SECTOR_NO)
        PUSH HL
        LD HL, (CURRENT_TRACK_NO)
        LD DE, DPB00_OFF
        ADD HL, DE
        LD A, DPB00_SPT
        CALL MUL16_8
        POP DE
        CALL ADD24_16

        ; Set SD Card address to write
        ;  (A,HL) * 128
        ;  aaaaaaaa hhhhhhhh lllllll l000000
        ;             HIGH     MID     LOW
        SRL A
        RRC H
        LD A, H
        OUT (DISK_WRITEPOS_H), A
        RRC L
        LD A, L
        OUT (DISK_WRITEPOS_M), A
        LD A,0
        ADC A 
        OUT (DISK_WRITEPOS_L), A

        ; Write
        OUT (DISK_WRITE), A
IS_WRITE_DONE:
        IN A, (DISK_WRITE_STS)
        OR 1
        JR NZ, IS_WRITE_DONE

        OR 2
        RET Z                   ; Success A=0
        SRL A                   ; Error A=1
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

        ; TODO: Translation
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

;================================================
; 24bit adder
;   A,HL + DE => A,HL
;================================================
ADD24_16:
        ADD HL, DE
        ADC A, 0
        RET

;================================================
; 16 * 8 bit multipler
;   HL * A => A,HL
;================================================
MULT16_8:
        LD B, A
        CALL  MUL16_8
        BIT 7, B
        RET Z
        NEG
        RET

MUL16_8:
        PUSH BC
        LD B, 8
        LD IX, 0
        LD C, 0
        LD DE, 0

MUL16_8_LOOP:
        RRCA
        JR NC, MUL16_8_SKIP

        PUSH DE
        PUSH HL
        POP  DE

        ADD IX, DE
        POP DE

        PUSH AF
        LD A, E
        ADD A, C
        LD E, A
        POP AF

MUL16_8_SKIP:
        ADD HL, HL
        RL C
        DJNZ MUL16_8_LOOP

        PUSH IX
        POP HL
        LD A, E

        POP BC
        RET

;******************************************************************
;       DPH: Disk Parameter Header
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
;       DPB: Disk Parameter Block
;
DPB00_SPT       .equ    16
DPB00_OFF       .equ    2
DPB00:  .dw    DPB00_SPT       ; SPT: sectors per track
        .db    3               ; BSH: block shift factor. sector in a block 128*2^n
        .db    7               ; BLM: block length mask.  sector no. in a block - 1
        .db    0               ; EXM: extent mask
        .dw    255             ; DSM: disk size max (number of blocks-1).
        .dw    127             ; DRM: directory size max (max file name no.-1)
        .dw    0xF000          ; AL0, AL1: storage for first bytes of bit map (dir space used).
        .dw    16              ; CKS: check sum vector size
        .dw    DPB00_OFF       ; OFF: offset. first usable track number.
;
;       Directory buffer (128bytes)
;
DIRB00: .ds     128
;
;       Check sum vector table (CKS in DPB bytes)
;
CSV00:  .ds     16
;
;       Allocation vector table (2bytes)
;
ALV00:  .dw     0

;******************************************************************
;   DMA buffer
;******************************************************************
DMABUF: .ds     512


