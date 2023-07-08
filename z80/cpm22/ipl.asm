;
; CP/M BIOS for Z80ATmega128
;
MEM	        .equ	62		; 62k CP/M
CCP_ENTRY   .equ    0x3400+(MEM-20)*1024
BIOS_ENTRY  .equ    CCP_ENTRY+0x1600

    .area IPL (ABS)
    .org 0x0000

    JP BIOS_ENTRY
