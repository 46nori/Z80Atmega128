;******************************************************************
;       CP/M-80 Version 2.2 BIOS for Z80ATmega128
;       Copyright (C) 2023 46nori
;******************************************************************
MEM	        .equ    62      ; 62K CP/M

CCP_ENTRY       .equ    (MEM-7)*1024
BDOS_ENTRY      .equ    CCP_ENTRY+0x800
BIOS_ENTRY      .equ    CCP_ENTRY+0x1600
CCP_BDOS_LENGTH .equ    0x1600
VECT_TABLE      .equ    (BIOS_ENTRY + 0x100) & 0xff00
IOBYTE          .equ    0x0003
LOGIN_DISK_NO   .equ    0x0004

; BIOS parameter
NUM_OF_DISKS    .equ    5       ; Number of supported disks

; Emulated I/O address 
PORT_CONIN      .equ    0x00    ; Get a character from CONIN
PORT_CONIN_STS  .equ    0x01    ; Check CONIN status
PORT_CONIN_BUF  .equ    0x02    ; Get Buffer size / Flush CONOUT
PORT_CONIN_INT  .equ    0x03    ; Interrupt setting of CONIN
PORT_CONOUT     .equ    0x05    ; Send a character to CONOUT
PORT_CONOUT_STS .equ    0x06    ; Check CONOUT status
PORT_CONOUT_BUF .equ    0x07    ; Get Buffer size / Flush CONOUT
PORT_CONOUT_INT .equ    0x08    ; Interrupt setting of CONOUT
PORT_SELDSK     .equ    0x0A    ; Select DISK
PORT_DSKSTS     .equ    0x0A    ; Check selected DISK status
PORT_DSKWRPOS   .equ    0x0B    ; Write position of DISK
PORT_DSKWRBUF   .equ    0x0C    ; Source buffer address in Write
PORT_DSKWRLEN   .equ    0x0D    ; Data length to Write
PORT_DSKWR      .equ    0x0E    ; Write DISK / Check status
PORT_DSKWR_INT  .equ    0x0F    ; Interrupt setting of DISK Write
PORT_DSKRDPOS   .equ    0x10    ; Read position of DISK
PORT_DSKRDBUF   .equ    0x11    ; Destination buffer address in Read
PORT_DSKRDLEN   .equ    0x12    ; Data length to Read
PORT_DSKRD      .equ    0x13    ; Read DISK / Check status
PORT_DSKRD_INT  .equ    0x14    ; Interrupt setting of DISK Read
PORT_DBG_INT    .equ    0x1D    ; Interrupt setting for resumption from HALT
PORT_DBG_INFO   .equ    0x1E    ; Send break point address
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
DEBUGGER:
        DI
        LD (SP_ADR), SP         ; (12)
        LD (DBG_HL), HL         ; Save HL
        EX (SP), HL             ; Get resume address
        DEC HL
        LD (BP_ADR), HL         ; (13)
        EX (SP), HL             ; Set resume address - 1
        POP HL                  ; SP = SP-1
        LD HL, (DBG_HL)         ; Restore HL

        ; save registers
        LD SP, REGS   
        PUSH IY                 ; (1) 
        PUSH IX                 ; (2)
        PUSH HL                 ; (3)
        PUSH DE                 ; (4)
        PUSH BC                 ; (5)
        PUSH AF                 ; (6)
        EX AF, AF'
        PUSH AF                 ; (7)
        EXX
        PUSH HL                 ; (8)
        PUSH DE                 ; (9)
        PUSH BC                 ; (10)
        LD (DBG_A), A           ; Save register
        LD A, I
        LD (REGS), A            ; (11)

        LD (DBG_HL), HL         ; Save registers
        LD (DBG_BC), BC
        LD (DBG_DE), DE

        IN A, (PORT_DBG_INFO)   ; Reset sequencer
        LD HL, DEBUG_INFO_END - 1
        LD B,  DEBUG_INFO_END - DEBUG_INFO_BEGIN
        LD C,  PORT_DBG_INFO
        OTDR                    ; Send debug info

        LD HL, (SP_ADR)
        LD DE, 2*3-1
        ADD HL, DE
        LD B, 2*3
        OTDR                    ; Send 3 level STACK data

        LD A,  (DBG_A)          ; Restore registers
        LD BC, (DBG_BC)
        LD DE, (DBG_DE)
        LD HL, (DBG_HL)
        EXX

        LD SP, (SP_ADR)         ; Restore SP
;        EI
        HALT                    ; Wait for INT 4
        RET                     ; Resume

DBG_A:  .db     0
DBG_BC: .dw     0
DBG_DE: .dw     0
DBG_HL: .dw     0

;
; Debug information area
;
DEBUG_INFO_BEGIN:
        .dw     0               ; (10) BC'
        .dw     0               ; (9)  DE'
        .dw     0               ; (8)  HL'
        .dw     0               ; (7)  AF'
        .dw     0               ; (6)  AF
        .dw     0               ; (5)  BC
        .dw     0               ; (4)  DE
        .dw     0               ; (3)  HL
        .dw     0               ; (2)  IX
        .dw     0               ; (1)  IY
REGS:   .db     0               ; (11) I
SP_ADR: .dw     0               ; (12) SP before break
BP_ADR: .dw     0               ; (13) breakpoint address
DEBUG_INFO_END:

;******************************************************************
;   Interrupt vector table (Must be 256 bytes alignment)
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
        LD HL, IS_CONOUT_FREE
        LD (HL), 1
        POP HL
        EI
        RETI
IS_CONOUT_FREE:
        .db     0               ; 0: full / 1: free

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
        LD SP, CCP_ENTRY                ; Set SP to ensure initialization
        LD A, VECT_TABLE >> 8           ; Mode2 interrupt setting
        LD I, A
        IM 2

        ;
        ; Init emulated I/O device
        ;
        ; Interrupt setting
        XOR A
        OUT (PORT_DSKRD_INT), A         ; INT 0
        INC A
        OUT (PORT_DSKWR_INT), A         ; INT 1
        INC A
        OUT (PORT_CONOUT_INT), A        ; INT 2
        INC A
        OUT (PORT_CONIN_INT), A         ; INT 3
        INC A
        OUT (PORT_DBG_INT), A           ; INT 4
        ; Init console I/O
        OUT (PORT_CONIN_BUF), A         ; Flush CONIN
        OUT (PORT_CONOUT_BUF), A        ; Flush CONOUT

        EI

        LD HL, BOOT_MSG                 ; Boot message
        CALL PRINT_STR

        CALL INIT_RD_CACHE              ; Init disk read
        CALL INIT_WRT_PENDING           ; Init disk write
        CALL LOAD_CCP_BDOS              ; Load CCP + BDOS
        CALL INIT_SYSTEM_AREA           ; Init system area

        XOR A
        LD (IOBYTE), A                  ; Init IOBYTE

        LD (LOGIN_DISK_NO), A           ; Set A:(0) to login disk
        LD C, A

        ; Start CP/M
        LD SP, CCP_ENTRY                ; Init SP
        JP CCP_ENTRY + 3                ; Exec CCP + 3

BOOT_MSG:
        .str    "\r\n"
        .str    "62K "
        .str    "CP/M-80 Ver2.2 on Z80ATmega128\r\n"
        .str    "BIOS Copyright (C) 2023 by 46nori\r\n"
        .db     0

;******************************************************************
;   01: Warm Boot
;       IN : None
;       OUT: C = default DISK number
;******************************************************************
WBOOT:
        LD SP, CCP_ENTRY                ; Set SP to ensure reload

        LD HL, REBOOT_MSG               ; Reboot message
        CALL PRINT_STR

        CALL INIT_RD_CACHE              ; Init disk read
        CALL INIT_WRT_PENDING           ; Init disk write
        CALL LOAD_CCP_BDOS              ; Load CCP + BDOS
        CALL INIT_SYSTEM_AREA           ; Init system area

        LD A, (LOGIN_DISK_NO)           ; Restore login disk
        LD C, A

        ; Start CP/M
        LD SP, CCP_ENTRY                ; Init SP
        JP CCP_ENTRY                    ; Exec CCP

REBOOT_MSG:
        .str    "\r\nReload CCP+BDOS.\r\n"
        .db     0

;================================================
; Load CCP + BDOS from boot disk
;================================================
LOAD_CCP_BDOS:
        ; Open DISK A:
        LD C, 0
        CALL SELDSK

        ; Set DMA address
        LD BC, 0x0080
        CALL SETDMA

        ; Set load address
        LD C, PORT_DSKRDBUF
        IN A, (C)                       ; Reset sequencer
        LD HL, CCP_ENTRY
        OUT (C), H
        OUT (C), L

        ; Set DISK read position
        LD C, PORT_DSKRDPOS
        IN A, (C)                       ; Reset sequencer
        XOR A
        OUT (C), A
        OUT (C), A
        OUT (C), A
        OUT (C), A

        ; Set DISK read length (size of CPP+BDOS)
        LD C, PORT_DSKRDLEN
        IN A, (C)                       ; Reset sequencer
        LD HL, CCP_BDOS_LENGTH
        OUT (C), H
        OUT (C), L

        ; Load CPP+BDOS
        CALL DISK_READ_SUB
        OR A
        RET Z                           ; Success

        ; System boot error
        OUT (PORT_CONOUT_BUF), A        ; Flush CONOUT
        LD HL, BOOT_ERROR_MSG
BOOT_ERROR_PRINT:
        LD A, (HL)
        OR A
        JR Z, BOOT_ERROR_HALT
        OUT (PORT_CONOUT), A
        INC HL
        JR BOOT_ERROR_PRINT
BOOT_ERROR_HALT:
        HALT
        JR BOOT_ERROR_HALT

BOOT_ERROR_MSG:
        .str    "\r\nSystem HALT due to CCP+BDOS load error.\r\n"
        .db     0

;================================================
; Init jump table of system area
;================================================
INIT_SYSTEM_AREA:
        LD A, 0xC3                      ; 'JP'

        ; Set 'JP _WBOOT' at 0x0000
        LD (0x0000), A
        LD HL, _WBOOT
        LD (0x0001), HL

        ; Set 'JP BDOS_ENTRY + 6' at 0x0005
        LD (0x0005), A
        LD HL, BDOS_ENTRY + 6 
        LD (0x0006), HL

        ; Set 'JP DEBUG_ENTRY' at 0x0038 for debugger
        LD (0x0038), A
        LD HL, DEBUGGER
        LD (0x0039), HL
        
        RET

;******************************************************************
;   02: Get status of CON:
;       IN : None
;       OUT: A = 0x00 - Not available
;                0xFF - Available
;******************************************************************
CONST:
        IN A, (PORT_CONIN_STS)
        OR A
        RET Z
        LD A, 0xFF
        RET

;******************************************************************
;   03: Input a character from CON: (Never return unless input)
;       IN : None
;       OUT: A = Received character
;******************************************************************
CONIN:
        XOR A
        DI
        LD (IS_CONIN_QUEUING), A        ; clear int flag
        IN A, (PORT_CONIN_STS)          ; check input
        EI
        OR A
        JR NZ, CONIN2
CONIN1:                                 ; wait for input
        LD A, (IS_CONIN_QUEUING)
        OR A
        JR Z, CONIN1
CONIN2:
        IN A, (PORT_CONIN)              ; read a character
        RET

;******************************************************************
;   04: Output a character to CON:
;       IN:  C = Character to output
;       OUT: None
;******************************************************************
CONOUT:
        PUSH AF
        DI
        IN A, (PORT_CONOUT_STS)
        CP 64
        JR C, CONOUT2
        ; wait until buffer is free enough
        XOR A
        LD (IS_CONOUT_FREE), A          ; clear int flag
        EI
CONOUT1:
        LD A, (IS_CONOUT_FREE)
        OR A
        JR Z, CONOUT1
CONOUT2:
        EI
        LD A, C
        OUT (PORT_CONOUT), A
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
        PUSH BC
        LD BC, 0
        CALL SETTRK
        POP BC
        RET

;******************************************************************
;   09: Select DISK
;       IN:  C  = DISK number
;       OUT: HL = DPH address - Success
;               = 0x0000      - Error
;******************************************************************
SELDSK:
        PUSH AF
        CALL WRITE_FLUSH

        LD A, NUM_OF_DISKS - 1
        CP C
        JR C, DISK_ERROR
        CALL INIT_RD_CACHE

        LD A, C
        OUT (PORT_SELDSK), A    ; Open DISK
        IN A, (PORT_DSKSTS)     ; Check DISK status
        OR A
        JR NZ, DISK_ERROR

        ; Success
        PUSH DE
        PUSH IX
        LD A, C
        ADD A, A
        LD D, 0
        LD E, A
        LD IX, DRIVE_TABLE
        ADD IX, DE
        LD H, (IX + 1)          ; Return HL = DPH
        LD L, (IX + 0)
        POP IX
        POP DE
        POP AF
        RET

        ; Error
DISK_ERROR:
        ; Force the login disk to A:.
        XOR A
        LD (LOGIN_DISK_NO), A
        LD C, A
        OUT (PORT_SELDSK), A    ; Open DISK A:

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
        CALL WRITE_FLUSH
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
        LD A, (IS_RD_CACHED)
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
        ;     HIGH                       LOW
        IN A, (PORT_DSKRDPOS)   ; Reset sequencer
        SRL E
        LD A, E
        OUT (PORT_DSKRDPOS), A
        RR H
        LD A, H
        OUT (PORT_DSKRDPOS), A
        RR L
        LD A, L
        OUT (PORT_DSKRDPOS), A
        LD A,0
        RRA
        OUT (PORT_DSKRDPOS), A

        ; Read a SD card sector (512bytes)
        IN A, (PORT_DSKRDBUF)   ; Reset sequencer
        LD HL, DMABUF
        LD A, H
        OUT (PORT_DSKRDBUF), A
        LD A, L
        OUT (PORT_DSKRDBUF), A

        ; Set read length (512byte)
        IN A, (PORT_DSKRDLEN)   ; Reset sequencer
        LD A, 0x02
        OUT (PORT_DSKRDLEN), A
        LD A, 0x00
        OUT (PORT_DSKRDLEN), A

        ; READ
RETRY_READ:
        CALL DISK_READ_SUB
        CP 4
        JR Z, RETRY_READ        ; retry if rejected
        CP 2
        JR Z, READ_ERROR

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
        LD (IS_RD_CACHED), A    ; Set cache flag
        XOR A                   ; Success A=0
        RET

READ_ERROR:
        XOR A
        LD (IS_RD_CACHED), A    ; Clear cache flag
        INC A                   ; Error A=1
        RET

CACHE_SECTOR_NO:                ; sector # of the 1st DMA buffer
        .db     0, 0, 0, 0
IS_RD_CACHED:
        .db     0               ; read cache flag

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
;  Init read cache
;================================================
INIT_RD_CACHE:
        PUSH AF
        XOR A
        LD (IS_RD_CACHED), A
        POP AF
        RET

;================================================
; DISK READ
;  OUT: A = disk status
;           0: Success
;           1: Reading
;           2: Error
;           4: Rejected
;================================================
DISK_READ_SUB:
        XOR A
        DI
        LD (IS_READ_DONE), A    ; reset flag
        OUT (PORT_DSKRD), A     ; read disk
        EI

WAIT_READ_COMPLETE:
        LD A, (IS_READ_DONE)
        OR A
        JR Z, WAIT_READ_COMPLETE
        IN A, (PORT_DSKRD)      ; Check read status
        RET

;******************************************************************
;   14: Write a record to DISK
;       IN:  C = Deblocking code
;                0 - Write can be deferred
;                1 - Write must be immediate
;                2 - Write can be deferred, no pre-read is necessary.
;       OUT: A = 0x00 - Success
;                0x01 - Unrecoverable error
;                0x02 - Read only
;                0xff - Media changed
;******************************************************************
WRITE:
        PUSH BC
        PUSH DE
        PUSH HL
        PUSH IX
        CALL WRITE_SUB
        POP IX
        POP HL
        POP DE
        POP BC
        RET

WRITE_SUB:
        ; Clear read cache
        CALL INIT_RD_CACHE

        PUSH BC

        ;       offset  index
        ; DMABUF[  0] ->  0
        ;       [128] ->  1
        ;       [256] ->  2
        ;       [384] ->  3
        ;
        ;       abssec = TRACK * SPT + SECTOR
        ;       index  = abssec % 4
        ;       offset = DMABUF + index * 128

        ; DE,HL = abssec
        LD HL, (CURRENT_TRACK_NO)
        LD DE, DPB00_SPT
        CALL MUL16_16
        LD BC, (CURRENT_SECTOR_NO)
        CALL ADD32_16

        ; A = index
        LD A, L
        AND 3

        ; HL = offset
        LD B, A
        INC B
        LD HL, DMABUF - 128
        LD DE, 128
WRTSUB_0:
        ADD HL, DE
        DJNZ WRTSUB_0
        PUSH HL

        ; Copy DMA data to offset in BIOS  buffer
        EX DE, HL
        LD HL, (DMA_ADRS)
        LD BC, 128
        LDIR

        ; update write count
        LD HL, WRT_COUNT
        INC (HL)

        POP HL                  ; HL = offset
        POP BC                  ;  C = Deblocking code
        LD B, A                 ;  B = index

        ; already in write pending?
        LD A, (IS_WRT_PENDING)
        OR A
        JR NZ, WRTSUB_1         ; yes, keep pending state

        ; Start write pending
        LD A, 1                 ; set pending flag
        LD (IS_WRT_PENDING), A

        ; save buffer offset
        LD (WRT_OFFSET), HL

        ; save track and sector
        LD HL, (CURRENT_TRACK_NO)
        LD (WRT_TRACK_NO), HL
        LD HL, (CURRENT_SECTOR_NO)
        LD (WRT_SECTOR_NO), HL
 
WRTSUB_1:
        ; Check if write immediate is necessary?
        LD A, B
        CP 3                    ; if index == 3
        JR Z, DO_WRITE
        LD A, C                 ; if C == 1
        CP 1
        JR Z, DO_WRITE

        XOR A                   ; Success on pending
        RET

DO_WRITE:
        ;
        ; Set SD Card address to write
        ;  (DE, HL) = TRACK * SPT + SECTOR
        LD HL, (WRT_TRACK_NO)
        LD DE, DPB00_SPT
        CALL MUL16_16
        LD BC, (WRT_SECTOR_NO)
        CALL ADD32_16
        ;
        ;  (DE,HL) * 128
        ;  0eeeeeee ehhhhhhh hlllllll l0000000
        ;    HIGH                        LOW
        IN A, (PORT_DSKWRPOS)   ; Reset sequencer
        SRL E
        LD A, E
        OUT (PORT_DSKWRPOS), A
        RR H
        LD A, H
        OUT (PORT_DSKWRPOS), A
        RR L
        LD A, L
        OUT (PORT_DSKWRPOS), A
        LD A, 0
        RRA
        OUT (PORT_DSKWRPOS), A

        ;
        ; Set source buffer address
        ;
        LD C, PORT_DSKWRBUF
        IN A, (C)               ; Reset sequencer
        LD HL, (WRT_OFFSET)
        OUT (C), H
        OUT (C), L

        ;
        ; Set write length
        ;   = (WRT_COUNT) * 128
        ;
        LD HL, 0
        LD BC, 128
        LD A, (WRT_COUNT)
DO_WRITE_0:
        ADD HL, BC
        DEC A
        JR NZ, DO_WRITE_0
        LD C, PORT_DSKWRLEN
        IN A, (C)               ; Reset sequencer
        OUT (C), H
        OUT (C), L

        ; Clear pending state
        CALL INIT_WRT_PENDING

        ; Write sectors
RETRY_WRITE:
        ; WRITE
        CALL DISK_WRITE_SUB
        CP 4
        JR Z, RETRY_WRITE       ; retry if rejected
        CP 2
        JR Z, WRITE_ERROR
        XOR A
        RET                     ; Success on write
WRITE_ERROR:
        LD A, 1                 ; Error on write
        RET

;  Pending state
IS_WRT_PENDING:
        .db     0               ; 1 if under pending
WRT_COUNT:
        .db     0               ; peding num of sectors
WRT_OFFSET:
        .dw     DMABUF          ; head address of buffer
WRT_TRACK_NO:
        .dw     0               ; head track to write
WRT_SECTOR_NO:
        .dw     0               ; head sector to write

;================================================
;  Flush sectors in pending write
;       OUT: A = 0x00 - Success
;                0x01 - Unrecoverable error
;                0x02 - Read only
;                0xff - Media changed
;================================================
WRITE_FLUSH:
        LD A, (IS_WRT_PENDING)
        OR A
        RET Z
        PUSH BC
        PUSH DE
        PUSH HL
        CALL DO_WRITE
        POP HL
        POP DE
        POP BC
        RET

;================================================
;  Init pending write state
;================================================
INIT_WRT_PENDING:
        PUSH AF
        XOR A
        LD (IS_WRT_PENDING), A
        LD (WRT_COUNT), A
        POP AF
        RET

;================================================
; DISK WRITE
;  OUT: A = disk status
;           0: Success
;           1: Writing
;           2: Error
;           4: Rejected
;================================================
DISK_WRITE_SUB:
        XOR A
        DI
        LD (IS_WRITE_DONE), A   ; reset flag
        OUT (PORT_DSKWR), A
        EI

WAIT_WRITE_COMPLETE:
        LD A, (IS_WRITE_DONE)
        OR A
        JR Z, WAIT_WRITE_COMPLETE
        IN A, (PORT_DSKWR)
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
;******************************************************************
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
        .db     0x80            ; AL0: storage for first bytes of bit map (dir space used).
        .db     0x00            ; AL1: storage for first bytes of bit map (dir space used).
        .dw     DPB00_CKS       ; CKS: check vector table size
        .dw     1               ; OFF: offset. first usable track number.

;******************************************************************
;       Drive Table (for NUM_OF_DISKS)
;******************************************************************
DRIVE_TABLE:
        .dw     DPH00           ; A:
        .dw     DPH01           ; B:
        .dw     DPH02           ; C:
        .dw     DPH03           ; D:
        .dw     DPH04           ; E:

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

DPH01:
        .dw     0               ; XLT: Translation Table address (0 if no table)
        .dw     0               ; SPA: scratch pad area 1
        .dw     0               ; SPA: scratch pad area 2
        .dw     0               ; SPA: scratch pad area 3
        .dw     DIRB01          ; DIRB: Directory Buffer address
        .dw     DPB00           ; DPB: Disk Parameter Block address
        .dw     CSV01           ; CSV: Check sum vector address
        .dw     ALV01           ; ALV: Allocation(bit map) vector address

DPH02:
        .dw     0               ; XLT: Translation Table address (0 if no table)
        .dw     0               ; SPA: scratch pad area 1
        .dw     0               ; SPA: scratch pad area 2
        .dw     0               ; SPA: scratch pad area 3
        .dw     DIRB02          ; DIRB: Directory Buffer address
        .dw     DPB00           ; DPB: Disk Parameter Block address
        .dw     CSV02           ; CSV: Check sum vector address
        .dw     ALV02           ; ALV: Allocation(bit map) vector address

DPH03:
        .dw     0               ; XLT: Translation Table address (0 if no table)
        .dw     0               ; SPA: scratch pad area 1
        .dw     0               ; SPA: scratch pad area 2
        .dw     0               ; SPA: scratch pad area 3
        .dw     DIRB03          ; DIRB: Directory Buffer address
        .dw     DPB00           ; DPB: Disk Parameter Block address
        .dw     CSV03           ; CSV: Check sum vector address
        .dw     ALV03           ; ALV: Allocation(bit map) vector address

DPH04:
        .dw     0               ; XLT: Translation Table address (0 if no table)
        .dw     0               ; SPA: scratch pad area 1
        .dw     0               ; SPA: scratch pad area 2
        .dw     0               ; SPA: scratch pad area 3
        .dw     DIRB04          ; DIRB: Directory Buffer address
        .dw     DPB00           ; DPB: Disk Parameter Block address
        .dw     CSV04           ; CSV: Check sum vector address
        .dw     ALV04           ; ALV: Allocation(bit map) vector address

;       Directory buffer (128bytes)
DIRB00: .ds     128
DIRB01: .ds     128
DIRB02: .ds     128
DIRB03: .ds     128
DIRB04: .ds     128

;       Check vector table (CKS in DPB bytes)
CSV00:  .ds     DPB00_CKS
CSV01:  .ds     DPB00_CKS
CSV02:  .ds     DPB00_CKS
CSV03:  .ds     DPB00_CKS
CSV04:  .ds     DPB00_CKS

;       Allocation vector table (DSM/8+1 bytes)
ALV00:  .ds     DPB00_DSM / 8 + 1
ALV01:  .ds     DPB00_DSM / 8 + 1
ALV02:  .ds     DPB00_DSM / 8 + 1
ALV03:  .ds     DPB00_DSM / 8 + 1
ALV04:  .ds     DPB00_DSM / 8 + 1

;******************************************************************
;   DMA buffer
;******************************************************************
DMABUF: .ds     512
