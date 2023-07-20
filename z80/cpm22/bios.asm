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
PORT_CONIN      .equ    0x00    ; Get a character from CONIN
PORT_CONIN_STS  .equ    0x01    ; Check CONIN status
PORT_CONIN_INT  .equ    0x03    ; Interrupt setting of CONIN
PORT_CONOUT     .equ    0x05    ; Send a character to CONOUT
PORT_CONOUT_STS .equ    0x06    ; Check CONOUT status
PORT_CONOUT_INT .equ    0x08    ; Interrupt setting of CONOUT
PORT_SELDSK     .equ    0x0A    ; Select DISK
PORT_DSKSTS     .equ    0x0A    ; Check selected DISK status
PORT_DSKWRPOS_L .equ    0x0B    ; Write position(L) of DISK
PORT_DSKWRPOS_M .equ    0x0C    ; Write position(M) of DISK
PORT_DSKWRPOS_H .equ    0x0D    ; Write position(H) of DISK
PORT_DSKWRBUF_L .equ    0x0E    ; Source buffer address(L) in Write
PORT_DSKWRBUF_H .equ    0x0F    ; Source buffer address(H) in Write
PORT_DSKWRLEN_L .equ    0x10    ; Data size(L) to Write
PORT_DSKWRLEN_H .equ    0x11    ; Data size(H) to Write
PORT_DSKWR      .equ    0x12    ; Write to DISK
PORT_DSKWR_STS  .equ    0x12    ; Check Write DISK status
PORT_DSKWR_INT  .equ    0x13    ; Interrupt setting of DISK Write
PORT_DSKRDPOS_L .equ    0x14    ; Read position (L) of DISK
PORT_DSKRDPOS_M .equ    0x15    ; Read position (M) of DISK
PORT_DSKRDPOS_H .equ    0x16    ; Read position (H) of DISK
PORT_DSKRDBUF_L .equ    0x17    ; Destination buffer address(L) in Read
PORT_DSKRDBUF_H .equ    0x18    ; Destination buffer address(H) in Read
PORT_DSKRDLEN_L .equ    0x19    ; Data size(L) to Read
PORT_DSKRDLEN_H .equ    0x1A    ; Data size(H) to Read
PORT_DSKRD      .equ    0x1B    ; Read from DISK
PORT_DSKRD_STS  .equ    0x1B    ; Check Read DISK status
PORT_DSKRD_INT  .equ    0x1C    ; Interrupt setting of DISK Read
PORT_LED        .equ    0x1F

; Interrupt settings of emulated device
ENABLE_CONOUT_INT   .equ    1   ; 1 if enable CONOUT interrupt

        .z80
        .area   BIOS (ABS)
        .org    BIOS_ENTRY

;******************************************************************
;   BIOS jump table
;******************************************************************
_CBOOT:         JP CBOOT
_WBOOT:         JP WBOOT
_CONST:         JP CONST
_CONIN:         JP CONIN
_CONOUT:        JP CONOUT
_LIST:          JP LIST
_PUNCH:         JP PUNCH
_READER:        JP READER
_HOME:          JP HOME
_SELDSK:        JP SELDSK
_SETTRK:        JP SETTRK
_SETSEC:        JP SETSEC
_SETDMA:        JP SETDMA
_READ:          JP READ
_WRITE:         JP WRITE
_PRSTAT:        JP PRSTAT
_SECTRN:        JP SECTRN

;******************************************************************
;   Interrupt vector table
;******************************************************************
        .org    VECT_TABLE
        .dw     ISR_00          ; INT 0 : DISK read  completed
        .dw     ISR_01          ; INT 1 : DISK write completed
        .dw     ISR_02          ; INT 2 : CONOUT

;******************************************************************
;   Interrupt handler
;******************************************************************
;
;       DISK READ complete
;
ISR_00:
        PUSH HL
        LD HL, IS_READ_DONE
        LD (HL), 1
        POP HL
        EI
        RETI
IS_READ_DONE:
        .db 0                   ; 0:doing / 1:complate

;
;       DISK WRITE complete
;
ISR_01:
        PUSH HL
        LD HL, IS_WRITE_DONE
        LD (HL), 1
        POP HL
        EI
        RETI
IS_WRITE_DONE:
        .db     0               ; 0:doing / 1:complate

;
;       CONSOLE OUT
;
ISR_02:
        PUSH HL
        LD HL, IS_CONOUT_DONE
        LD (HL), 1
        POP HL
        EI
        RETI
IS_CONOUT_DONE:
        .db     0               ; 0: doing / 1: complete

;
;       Initialize interrupt
;
INIT_INTERRUPT:
        DI
        LD HL, VECT_TABLE
        LD A, H
        LD i, A
        IM 2

        XOR A
        OUT (PORT_DSKRD_INT), A         ; INT 0
        INC A
        OUT (PORT_DSKWR_INT), A         ; INT 1
.if ENABLE_CONOUT_INT
        INC A
        OUT (PORT_CONOUT_INT), A        ; INT 2
.else
        LD A, 128
        OUT (PORT_CONOUT_INT), A        ; Disable INT
.endif
        EI
        RET

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
;       IN : None
;       OUT: None
;******************************************************************
CBOOT:
        LD SP, CCP_ENTRY

        ; Interrupt setting of emulated device
        CALL INIT_INTERRUPT

        ; Boot message
        LD HL, BOOT_MSG
        CALL PRINT_STR

        LD A, 7
        CALL LED_OFF

        ; IOBYTE
        LD A, 0x01              ; CRT: for CONSOLE 
        LD (IOBYTE), A

        ; Default deive
        LD C, 0

        JP CCP_ENTRY            ; Exec CCP

BOOT_MSG:
        .str    "\r\n"
        .str    "CP/M-80 Ver2.2 on Z80ATmega128\r\n"
        .db     0

;******************************************************************
;   01: Warm Boot
;       IN : None
;       OUT: C : default DISK number
;******************************************************************
WBOOT:
        ; Interrupt setting of emulated device
        CALL INIT_INTERRUPT

        ; Reload CCP and BDOS
        ;CALL LOAD_CCP_BDOS

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
        JP CCP_ENTRY + 3        ; Exec CCP with clear buffer

;******************************************************************
;   02: Get status of CON:
;       IN : None
;       OUT: A : 0 (Not available), 0xFF (Available)
;******************************************************************
CONST:
        IN A, (PORT_CONIN_STS)
        OR A
        RET Z
        LD A, 0xFF
        RET

;******************************************************************
;   03: Input a character from CON:
;       IN : None
;       OUT: A : Received character
;******************************************************************
CONIN:
        IN A, (PORT_CONIN)
        RET

;******************************************************************
;   04: Output a character to CON:
;       IN:  C : Character to output
;       OUT: None
;******************************************************************
CONOUT:
        PUSH AF
        LD A, C
        OUT (PORT_CONOUT), A
.if ENABLE_CONOUT_INT
WAIT_CONOUT:
        LD A, (IS_CONOUT_DONE)
        OR A
        JR Z, WAIT_CONOUT
        XOR A
        LD (IS_CONOUT_DONE), A
        POP AF
        RET
.else
WAIT_CONOUT:
        IN A, (PORT_CONOUT_STS)
        OR A
        JR NZ, WAIT_CONOUT
        POP AF
        RET
.endif

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
;       IN:  C : DISK number
;       OUT: HL: DPH address
;******************************************************************
SELDSK:
        PUSH AF
        LD A, C
        OUT (PORT_SELDSK), A
        IN A, (PORT_DSKSTS)
        OR A
        JR NZ, DISK_ERROR

        ; Success
        LD HL, DPH00            ; Set DPH
        LD A, C
        LD (DRIVE_NO), A        ; Save current drive
        XOR A
        LD (IS_CACHED), A       ; Clear read cache
        POP AF
        RET

DISK_ERROR:
        POP AF
        LD HL, 0
        RET

DRIVE_NO:
        .db     0

;******************************************************************
;   10: Set logical track number
;       IN:  BC : Track number
;       OUT: None
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
;       IN:  BC : Sector number
;       OUT: None
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
;       IN:  BC : Buffer address
;       OUT: None
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
;       IN:  None
;       OUT: 0x00 - Success
;            0x01 - Unrecoverable error
;            0xff - Media changed
;******************************************************************
READ:
        EXX
        ; Set destination buffer address
        LD HL, DMABUF
        LD A, H
        OUT (PORT_DSKRDBUF_H), A
        LD A, L
        OUT (PORT_DSKRDBUF_L), A

        ; Set read length (512byte)
        LD A, 0x02
        OUT (PORT_DSKRDLEN_H), A
        LD A, 0x00
        OUT (PORT_DSKRDLEN_L), A

        ; DE,HL = (TRACK + OFF) * SPT + SECTOR
        LD HL, (CURRENT_TRACK_NO)
        LD DE, DPB00_OFF
        ADD HL, DE
        LD DE, DPB00_SPT
        CALL MUL16_16
        LD IX, (CURRENT_SECTOR_NO)
        PUSH IX
        POP BC
        CALL ADD32_16

        ; Check cache
        LD IX, CACHE_SECTOR_NO
        LD A, D
        CP (IX + 0)
        JR NZ, MISHIT_CACHE
        LD A, E
        CP (IX + 1)
        JR NZ, MISHIT_CACHE
        LD A, H
        CP (IX + 2)
        JR NZ, MISHIT_CACHE
        LD A, L
        CP (IX + 3)
        JR NZ, MISHIT_CACHE

        ; Cache hit
        LD A, L
        AND 0x03
        LD B, A                 ; B = L % 4 (blocking factor)
        CALL READ_DMA_BUFFER
        EXX
        XOR A                   ; Success A=0
        RET                     ; Exit

MISHIT_CACHE:
        ; calc 512byte bundary sector
        LD B, L
        LD A, 0xf8
        AND L
        LD L, A                 ; L = L / 4 (truncation)

        LD A, 0x03
        AND B
        LD B, A                 ; B = L % 4 (blocking factor)

        ; Here, DE,HL = (TRACK + OFF) * SPT + SECTOR

        ; Set SD Card address to read
        ;  (DE,HL) * 128
        ;  0eeeeeee ehhhhhhh hlllllll l0000000
        ;             HIGH     MID      LOW
        SRL E
        RRC H
        LD A, H
        OUT (PORT_DSKRDPOS_H), A
        RRC L
        LD A, L
        OUT (PORT_DSKRDPOS_M), A
        LD A,0
        ADC A 
        OUT (PORT_DSKRDPOS_L), A

        ; Read a SD card sector (512bytes)
RETRY_READ:
        ; READ
        OUT (PORT_DSKRD), A
WAIT_READ_COMPLETE:
        ; Wait for complete interrupt
        LD A, (IS_READ_DONE)
        OR A
        JR Z, WAIT_READ_COMPLETE
        XOR A
        LD (IS_READ_DONE), A   ; reset flag

        ; Check read status
        IN A, (PORT_DSKRD_STS)
        BIT 1, A
        JR NZ, READ_ERROR
        BIT 2, A
        JR NZ, RETRY_READ       ; retry if rejected

        ; Read successfully and Copy buffer
        CALL READ_DMA_BUFFER

        ; save cache info
        LD IX, CACHE_SECTOR_NO
        LD (IX + 0), D
        LD (IX + 1), E
        LD (IX + 2), H
        LD (IX + 3), L
        LD A, 1
        LD (IS_CACHED), A       ; Set cache flag
        EXX
        XOR A                   ; Success A=0
        RET                     ; Exit

READ_ERROR:
        EXX
        XOR A
        LD (IS_CACHED), A       ; Clear cache flag
        INC A                   ; Error A=1
        RET                     ; Exit

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
        .db     0, 0, 0, 0
IS_CACHED:
        .db     0

;******************************************************************
;   14: Write a record to DISK
;       IN:  C=0 - Write can be deferred
;            C=1 - Write must be immediate
;            C=2 - Write can be deferred, no pre-read is necessary.
;       OUT: 0x00 - Success
;            0x01 - Unrecoverable error
;            0x02 - Read only
;            0xff - Media changed
;******************************************************************
WRITE:
        EXX

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
        OUT (PORT_DSKWRBUF_H), A
        LD A, L
        OUT (PORT_DSKWRBUF_L), A

        ; Set write length (128byte)
        XOR A
        OUT (PORT_DSKWRLEN_H), A
        LD A, 128
        OUT (PORT_DSKWRLEN_L), A

        ; DE,HL = (TRACK + OFF) * SPT + SECTOR
        LD HL, (CURRENT_TRACK_NO)
        LD DE, DPB00_OFF
        ADD HL, DE
        LD DE, DPB00_SPT
        CALL MUL16_16
        LD IX, (CURRENT_SECTOR_NO)
        PUSH IX
        POP BC
        CALL ADD32_16

        ; Set SD Card address to write
        ;  (DE,HL) * 128
        ;  0eeeeeee ehhhhhhh hlllllll l0000000
        ;             HIGH     MID      LOW
        SRL E
        RRC H
        LD A, H
        OUT (PORT_DSKWRPOS_H), A
        RRC L
        LD A, L
        OUT (PORT_DSKWRPOS_M), A
        LD A,0
        ADC A 
        OUT (PORT_DSKWRPOS_L), A

        EXX

        ; Write a SD card sector (512bytes)
RETRY_WRITE:
        ; WRITE
        OUT (PORT_DSKWR), A
WAIT_WRITE_COMPLETE:
        ; Wait for complete interrupt
        LD A, (IS_WRITE_DONE)
        OR A
        JR Z, WAIT_WRITE_COMPLETE
        XOR A
        LD (IS_WRITE_DONE), A   ; reset flag

        ; check write status
        IN A, (PORT_DSKWR_STS)
        BIT 1, A
        JR NZ, WRITE_ERROR
        BIT 2, A
        JR NZ, RETRY_WRITE      ; retry if rejected
        XOR A
        RET                     ; Success A=0

WRITE_ERROR:
        LD A, 1                 ; Error A=1
        RET

;******************************************************************
; 15: Check status of LST: (Not implemented)
;******************************************************************
PRSTAT:
        XOR A           ; Not ready
        RET

;******************************************************************
;   16: Translate logical sector to physical sector
;       IN:  BC : Sector number
;            DE ; Translation table
;       OUT: HL : Translated Sector number
;******************************************************************
SECTRN:
        ; No translation
        LD H, B
        LD L, C
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
;================================================
LED_ON:
        PUSH BC
        CPL
        LD C, A
        IN A, (PORT_LED)
        AND C
        OUT (PORT_LED), A
        POP BC
        RET
LED_OFF:
        PUSH BC
        LD C, A
        IN A, (PORT_LED)
        OR C
        OUT (PORT_LED), A
        POP BC
        RET

;================================================
; 16 * 16 bit multipler
;   DE * HL => DE,HL
;================================================
MUL16_16:
        PUSH AF
        PUSH BC
        PUSH IX
        LD B, H
        LD C, L
        PUSH DE
        POP IX
        LD HL, 0x0000
        LD A, 16
MUL1616_1:
        ADD HL, HL
        RL E
        RL D
        ADD IX, IX
        JR NC, MUL1616_2
        ADD HL, BC
        JR NC, MUL1616_2
        INC DE
MUL1616_2:
        DEC A
        JR  NZ, MUL1616_1
        POP IX
        POP BC
        POP AF
        RET

;================================================
; 32 + 16 bit adder
;   DE,HL + BC => DE,HL
;================================================
ADD32_16:
        ADD HL, BC
        LD BC, 0
        EX DE, HL
        ADC HL, BC
        EX DE, HL
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
DPB00_SPT      .equ     16
DPB00_OFF      .equ     2
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
