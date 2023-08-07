;******************************************************************
;       CP/M-80 Version 2.2 BIOS for Z80ATmega128
;       Copyright (C) 2023 46nori
;
;******************************************************************
MEM	        .equ    62              ; 62K CP/M

CCP_ENTRY       .equ    (MEM-7)*1024
BDOS_ENTRY      .equ    CCP_ENTRY+0x800
BIOS_ENTRY      .equ    CCP_ENTRY+0x1600
CCP_BDOS_LENGTH .equ    0x1600
VECT_TABLE      .equ    (BIOS_ENTRY + 0x100) & 0xff00
IOBYTE          .equ    0x0003
CURRENT_DISK    .equ    0x0004

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
PORT_DEBUG_INT  .equ    0x1D    ; Interrupt setting of Debugger
PORT_DEBUG_OP   .equ    0x1E    ; Get OP code of the break point
PORT_LED        .equ    0x1F    ; LED control

        .z80
        .area   BIOS (ABS)
        .org    BIOS_ENTRY

;******************************************************************
;   BIOS jump table
;******************************************************************
_BOOT:          JP BOOT
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
;   Debugger (RST 7 handler)
;******************************************************************
DEBUGGER_ENTRY:
        JP BREAKPOINT

BREAKPOINT:
        DI
        EX (SP),HL
        DEC HL
        LD (BREAKPOINT_ADR), HL
        EX (SP),HL              ; Set resume address
        EI
        PUSH AF
        LD A, (BREAKPOINT_ADR)
        OUT (PORT_DEBUG_OP), A  ; send low address of breakpoint
        POP AF
        HALT                    ; Wait for INT 4
        RET                     ; Reesume

BREAKPOINT_ADR:
        .dw     0

;******************************************************************
;   Interrupt vector table
;******************************************************************
        .org    VECT_TABLE
        .dw     ISR_00          ; INT 0 : DISK read  completed
        .dw     ISR_01          ; INT 1 : DISK write completed
        .dw     ISR_02          ; INT 2 : CONOUT
        .dw     ISR_03          ; INT 3 : CONIN
        .dw     ISR_04          ; INT 4 : Debugger

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
;       CONSOLE OUT complete
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
;       CONSOLE IN data queuing
;
ISR_03:
        PUSH HL
        LD HL, IS_CONIN_QUEUING
        LD (HL), 1
        POP HL
        EI
        RETI
IS_CONIN_QUEUING:
        .db     0               ; 0: no data / 1: queuing

;
;       Debugger
;
ISR_04: EI                      ; do nothing. Just release HALT.
        RETI

;******************************************************************
;   00: Cold Boot
;       IN : None
;       OUT: None
;******************************************************************
BOOT:
        DI
        LD SP, CCP_ENTRY

        ; Interrupt setting of emulated device
        LD HL, VECT_TABLE
        LD A, H
        LD I, A
        IM 2
        XOR A
        OUT (PORT_DSKRD_INT), A         ; INT 0
        INC A
        OUT (PORT_DSKWR_INT), A         ; INT 1
        INC A
        OUT (PORT_CONOUT_INT), A        ; INT 2
        INC A
        OUT (PORT_CONIN_INT), A         ; INT 3
        INC A
        OUT (PORT_DEBUG_INT), A         ; INT 4
        EI

        ; Boot message
        LD HL, BOOT_MSG
        CALL PRINT_STR

        CALL INIT_SYSTEM_AREA

        ; Init IOBYTE
        LD A,0x00
        LD (IOBYTE), A

        ; Set default drive as A:(0)
        XOR A
        LD (CURRENT_DISK), A
        LD C, A
        JP CCP_ENTRY+3          ; Exec CCP

BOOT_MSG:
        .str    "\r\n"
        .str    "CP/M-80 Ver2.2 on Z80ATmega128\r\n"
        .db     0

INIT_SYSTEM_AREA:
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

        ; Set 'JP DEBUG_ENTRY' at 0x0038 for debugger
        LD A, 0xC3
        LD (0x0038), A
        LD HL, DEBUGGER_ENTRY
        LD (0x0039), HL

        RET

;******************************************************************
;   01: Warm Boot
;       IN : None
;       OUT: C = default DISK number
;******************************************************************
WBOOT:
        ;
        ; Reload CCP and BDOS
        ;
        ; Open DISK
        LD C, 0
        CALL SELDSK

        ; Set DISK read position
        XOR A
        OUT (PORT_DSKRDPOS_H), A
        OUT (PORT_DSKRDPOS_M), A
        OUT (PORT_DSKRDPOS_L), A

        ; Set load address
        LD HL, CCP_ENTRY
        LD A, H
        OUT (PORT_DSKRDBUF_H), A
        LD A, L
        OUT (PORT_DSKRDBUF_L), A

        ; Set DISK read length (size of CPP+BDOS)
        LD HL, CCP_BDOS_LENGTH
        LD A, H
        OUT (PORT_DSKRDLEN_H), A
        LD A, L
        OUT (PORT_DSKRDLEN_L), A

        ; Load CPP+BDOS
        CALL DISK_READ_SUB
        OR A
        JR NZ, BOOT_ERROR
        LD HL, RELOAD_MSG
        CALL PRINT_STR

        CALL INIT_SYSTEM_AREA

        ; Set current DISK
        XOR A
        LD (IS_CACHED), A       ; Clear read cache
        LD A, (CURRENT_DISK)
        LD C, A

        ; Start CP/M
        LD SP, CCP_ENTRY        ; Init SP
        JP CCP_ENTRY            ; Exec CCP

        ; System boot error
BOOT_ERROR:
        LD HL, BOOT_ERROR_MSG
BOOT_ERROR_PRINT:
        LD A, (HL)
        OR A
        JR Z, BOOT_ERROR_HALT
        LD C, A
        OUT (PORT_CONOUT), A
        INC HL
        JR BOOT_ERROR_PRINT
BOOT_ERROR_HALT:
        HALT

BOOT_ERROR_MSG:
        .str    "System HALT due to CCP+BDOS load error.\r\n"
        .db     0
RELOAD_MSG:
        .str    "\r\nCCP+BDOS reloaded.\r\n"
        .db     0

;******************************************************************
;   02: Get status of CON:
;       IN : None
;       OUT: A = 0x00 - Not available
;                0xFF - Available
;******************************************************************
CONST:
.if 0
        IN A, (PORT_CONIN_STS)
        OR A
        RET Z
        LD A, 0xFF
        RET
.else
        PUSH HL
        XOR A                   ; A = 0
        LD HL, IS_CONIN_QUEUING
        CP (HL)
        POP HL
        RET Z
        CPL                     ; A = 0xFF
        RET
.endif

;******************************************************************
;   03: Input a character from CON:
;       IN : None
;       OUT: A = Received character
;******************************************************************
CONIN:
        IN A, (PORT_CONIN)
        PUSH AF
        IN A, (PORT_CONIN_STS)
        OR A
        JR NZ, CONIN_RET
        LD (IS_CONIN_QUEUING), A
CONIN_RET:
        POP AF
        RET

;******************************************************************
;   04: Output a character to CON:
;       IN:  C = Character to output
;       OUT: None
;******************************************************************
CONOUT:
        PUSH AF
        LD A, C
        OUT (PORT_CONOUT), A
WAIT_CONOUT:
        LD A, (IS_CONOUT_DONE)
        OR A
        JR Z, WAIT_CONOUT
        XOR A
        LD (IS_CONOUT_DONE), A
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
        LD A, 0x1A              ; Terminator
        RET

;******************************************************************
;   08: Return selected DISK to home position
;******************************************************************
HOME:
        RET

;******************************************************************
;   09: Select DISK
;       IN:  C  = DISK number
;       OUT: HL = DPH address - Success
;               = 0x0000      - Error
;******************************************************************
SELDSK:
        PUSH AF
        XOR A
        LD (IS_CACHED), A       ; Clear read cache

        LD A, C
        OUT (PORT_SELDSK), A    ; Open DISK
        IN A, (PORT_DSKSTS)     ; Check DISK status
        OR A
        JR NZ, DISK_ERROR

        ; Success
        LD HL, DPH00            ; Return HL = DPH
        POP AF
        RET
        ; Error
DISK_ERROR:
        LD HL, 0                ; Return HL = 0
        POP AF
        RET

;******************************************************************
;   10: Set logical track number
;       IN:  BC = Track number
;       OUT: None
;******************************************************************
SETTRK:
        LD (CURRENT_TRACK_NO), BC
        RET
CURRENT_TRACK_NO:
        .dw     0

;******************************************************************
;   11: Set logical sector number
;       IN:  BC = Sector number
;       OUT: None
;******************************************************************
SETSEC:
        LD (CURRENT_SECTOR_NO), BC
        RET
CURRENT_SECTOR_NO:
        .dw     0

;******************************************************************
;   12: Set data buffer address
;       IN:  BC = Buffer address
;       OUT: None
;******************************************************************
SETDMA:
        LD (DMA_ADRS), BC
        RET
DMA_ADRS:
        .dw     0

;******************************************************************
;   13: Read a record from DISK
;       IN:  None
;       OUT: A = 0x00 - Success
;                0x01 - Unrecoverable error
;                0xff - Media changed
;******************************************************************
READ:
        PUSH BC
        PUSH DE
        PUSH HL
        PUSH IX
        PUSH IY
        CALL READ_SUB
        POP IY
        POP IX
        POP HL
        POP DE
        POP BC
        RET

READ_SUB:
        ; DE,HL = TRACK * SPT + SECTOR
        LD HL, (CURRENT_TRACK_NO)
        LD DE, DPB00_SPT
        CALL MUL16_16
        LD BC, (CURRENT_SECTOR_NO)
        CALL ADD32_16

        ; Check cache
        LD A, (IS_CACHED)
        OR A
        JR Z, MISHIT_CACHE
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
        AND 0xFC                ; A = L / 4
        CP (IX + 3)
        JR NZ, MISHIT_CACHE

        ; Cache hit
        LD A, L
        AND 0x03
        LD B, A                 ; B = L % 4 (blocking factor)
        CALL READ_DMA_BUFFER    ; Return cache data
        XOR A                   ; Success A=0
        RET

MISHIT_CACHE:
        ; calc 512byte bundary sector
        LD A, L
        AND 0x03
        LD B, A                 ; B = L % 4 (blocking factor)

        LD A, L
        AND 0xFC
        LD L, A                 ; L = L / 4 (truncation)

        ; Here, DE,HL = (TRACK * SPT + SECTOR) & 0xfffffffc
        PUSH DE
        POP IX                  ; IX = DE
        PUSH HL
        POP IY                  ; IY = HL

        ; Set SD Card address to read
        ;  (DE,HL) * 128
        ;  0eeeeeee ehhhhhhh hlllllll l0000000
        ;             HIGH     MID      LOW
        SRL E
        RR H
        LD A, H
        OUT (PORT_DSKRDPOS_H), A
        RR L
        LD A, L
        OUT (PORT_DSKRDPOS_M), A
        LD A,0
        RRA
        OUT (PORT_DSKRDPOS_L), A

        ; Read a SD card sector (512bytes)
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

        ; READ
RETRY_READ:
        CALL DISK_READ_SUB
        BIT 1, A
        JR NZ, READ_ERROR
        BIT 2, A
        JR NZ, RETRY_READ       ; retry if rejected

        ; Copy buffer
        CALL READ_DMA_BUFFER

        ; save cache info
        PUSH IX
        POP DE
        PUSH IY
        POP HL
        LD IX, CACHE_SECTOR_NO
        LD (IX + 0), D
        LD (IX + 1), E
        LD (IX + 2), H
        LD (IX + 3), L
        LD A, 1
        LD (IS_CACHED), A       ; Set cache flag
        XOR A                   ; Success A=0
        RET

READ_ERROR:
        XOR A
        LD (IS_CACHED), A       ; Clear cache flag
        INC A                   ; Error A=1
        RET

CACHE_SECTOR_NO:                ; sector # of the 1st DMA buffer
        .db     0, 0, 0, 0
IS_CACHED:
        .db     0

;================================================
; Copy BIOS buffer to DMA buffer
;  IN: B = blocking factor (0,1,2,3)
;  OUT: (DMA_ADRS) <- (DMABUF+128*B) for 128 bytes
;================================================
READ_DMA_BUFFER:
        LD HL, DMABUF
        LD C, 0
        SRL B
        RR C                    ; BC = 128*B
        ADD HL, BC              ; HL = DMABUF+128*B
        LD DE, (DMA_ADRS)       ; DE = destination
        LD BC, 128
        LDIR
        RET

;================================================
; DISK READ
;  OUT: A = disk status
;================================================
DISK_READ_SUB:
        XOR A
        LD (IS_READ_DONE), A    ; reset flag
        OUT (PORT_DSKRD), A     ; read disk
WAIT_READ_COMPLETE:
        ; Wait for complete interrupt
        LD A, (IS_READ_DONE)
        OR A
        JR Z, WAIT_READ_COMPLETE
        IN A, (PORT_DSKRD_STS)  ; Check read status
        RET

;******************************************************************
;   14: Write a record to DISK
;       IN:  C = 0 - Write can be deferred
;            C = 1 - Write must be immediate
;            C = 2 - Write can be deferred, no pre-read is necessary.
;       OUT: A = 0x00 - Success
;                0x01 - Unrecoverable error
;                0x02 - Read only
;                0xff - Media changed
;******************************************************************
WRITE:
        PUSH BC
        PUSH DE
        PUSH HL

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

        ; DE,HL = TRACK * SPT + SECTOR
        LD HL, (CURRENT_TRACK_NO)
        LD DE, DPB00_SPT
        CALL MUL16_16
        LD BC, (CURRENT_SECTOR_NO)
        CALL ADD32_16

        ; Set SD Card address to write
        ;  (DE,HL) * 128
        ;  0eeeeeee ehhhhhhh hlllllll l0000000
        ;             HIGH     MID      LOW
        SRL E
        RR H
        LD A, H
        OUT (PORT_DSKWRPOS_H), A
        RR L
        LD A, L
        OUT (PORT_DSKWRPOS_M), A
        LD A,0
        RRA
        OUT (PORT_DSKWRPOS_L), A

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
        JR WRITE_EXIT           ; Success A=0

WRITE_ERROR:
        LD A, 1                 ; Error A=1

WRITE_EXIT:
        POP HL
        POP DE
        POP BC
        RET

;******************************************************************
; 15: Check status of LST: (Not implemented)
;******************************************************************
PRSTAT:
        XOR A           ; Not ready
        RET

;******************************************************************
;   16: Translate logical to physical sector (Not implemented)
;       IN:  BC = Sector number
;            DE = Translation table
;       OUT: HL = Translated Sector number
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
;  IN: HL : string terminated by 0x00
;================================================
PRINT_STR:
        PUSH AF
        PUSH BC
PRINT_STR_LOOP:
        LD A, (HL)
        OR A
        JR Z, PRINT_STR_END
        LD C, A
        CALL CONOUT
        INC HL
        JR PRINT_STR_LOOP
PRINT_STR_END:
        POP BC
        POP AF
        RET

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
DPH00:
        .dw     0               ; XLT: Translation Table address (0 if no table)
        .dw     0               ; SPA: scratch pad area 1
        .dw     0               ; SPA: scratch pad area 2
        .dw     0               ; SPA: scratch pad area 3
        .dw     DIRB00          ; DIRB: Directory Buffer address
        .dw     DPB00           ; DPB: Disk Parameter Block address
        .dw     CSV00           ; CSV: Check sum vector address
        .dw     ALV00           ; ALV: Allocation(bit map) vector address

;==================================================================
;       DPB: Disk Parameter Block
;
;       SSZ: 128 bytes per a sector 
;       BLS: Block size, (BLM+1) * SSZ
;       Total DISK size = (DSM+1) * BLS = MaxTracks * SPT * SSZ
;       N: Number of '1's in regard AL0,AL1 as bit fields
;
;           BLS  BSH  BLM         EXM             DRM
;                          (DSM<=255)(DSM>255)  
;       ==================================================
;          1024    3    7       0       -        32 x N
;          2048    4   15       1       0        64 x N
;          4096    5   31       3       1       128 x N
;          8192    6   63       7       3       256 x N
;         16384    7  127      15       7       512 x N
;
;       CKS: check sum vector size
;               (DRM + 1) / 4 if removable disk
;               0 if hard disk
;       AVL: Allocation vector size, DSM / 8 + 1
;==================================================================
DPB00_SPT       .equ    256
DPB00_DSM       .equ    1023
DPB00_DRM       .equ    255
DPB00_CKS       .equ    0       ; (DPB00_DRM + 1) / 4
DPB00:  .dw     DPB00_SPT       ; SPT: sectors per track
        .db     6               ; BSH: block shift factor. sector in a block SSZ*2^n
        .db     63              ; BLM: block length mask.  sector number in a block - 1
        .db     3               ; EXM: extent mask
        .dw     DPB00_DSM       ; DSM: disk size max (number of blocks-1).
        .dw     DPB00_DRM       ; DRM: directory size max (max file name no.-1)
        .dw     0x8000          ; AL0, AL1: storage for first bytes of bit map (dir space used).
        .dw     DPB00_CKS       ; CKS: check vector table size
        .dw     1               ; OFF: offset. first usable track number.

;==================================================================
;       Directory buffer (128bytes)
DIRB00: .ds     128

;       Check vector table (CKS in DPB bytes)
CSV00:  .ds     DPB00_CKS

;       Allocation vector table (DSM/8+1 bytes)
ALV00:  .ds     DPB00_DSM / 8 + 1

;******************************************************************
;   DMA buffer
;******************************************************************
DMABUF: .ds     512
