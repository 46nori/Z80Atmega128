;
; CP/M BIOS for Z80ATmega128
;
MEM         .equ        62		; 62K CP/M
CCP_ENTRY   .equ        (MEM-7)*1024
BIOS_ENTRY  .equ        CCP_ENTRY+0x1600

        .area IPL (ABS)
        .org 0x0000

        JP BIOS_ENTRY
