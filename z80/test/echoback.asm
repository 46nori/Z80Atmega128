VECT_TBL    .equ    0x0100

    .area TEST (ABS)
    .org 0x0000

    di
    ld sp, 0x0000
    ld hl, TABLE
    ld a, h
    ld i, a
    im 2
    ld hl, VECTNO
    ei

    ; RX1 int 0x10
    ld a, 0x10
    out (0x01), a

    ; reset interrupt count for debug
    xor a
    ld (INTCOUNT), a

;    call HELLO

LOOP:
    jp LOOP

;
; echo back
;
ECHO:
    ; copy interrupt vector for debug
    ld (hl), a

ECHO_LOOP:
    in a, (0x02)        ; read a character
    out (0x02), a       ; write a character
    in a, (0x03)        ; check remaining data
    or a
    jr nz, ECHO_LOOP
    ; Update interrupt count for debug

    ld a, (INTCOUNT)    ; increment interrupt count
    inc a
    ld (INTCOUNT), a

    ei                  ; !!!! IMPORTANT !!!!
    ret

    .org 0x0090
VECTNO:
    .db 0xaa
INTCOUNT:
    .db 0
;
; Interrupt vector table
;
    .org VECT_TBL
TABLE:
    .dw ISR_00, ISR_01, ISR_02, ISR_03, ISR_04, ISR_05, ISR_06, ISR_07
    .dw ISR_08, ISR_09, ISR_0A, ISR_0B, ISR_0C, ISR_0D, ISR_0E, ISR_0F
    .dw ISR_10, ISR_11, ISR_12, ISR_13, ISR_14, ISR_15, ISR_16, ISR_17
    .dw ISR_18, ISR_19, ISR_1A, ISR_1B, ISR_1C, ISR_1D, ISR_1E, ISR_1F
    .dw ISR_20, ISR_21, ISR_22, ISR_23, ISR_24, ISR_25, ISR_26, ISR_27
    .dw ISR_28, ISR_29, ISR_2A, ISR_2B, ISR_2C, ISR_2D, ISR_2E, ISR_2F
    .dw ISR_30, ISR_31, ISR_32, ISR_33, ISR_34, ISR_35, ISR_36, ISR_37
    .dw ISR_38, ISR_39, ISR_3A, ISR_3B, ISR_3C, ISR_3D, ISR_3E, ISR_3F
    .dw ISR_40, ISR_41, ISR_42, ISR_43, ISR_44, ISR_45, ISR_46, ISR_47
    .dw ISR_48, ISR_49, ISR_4A, ISR_4B, ISR_4C, ISR_4D, ISR_4E, ISR_4F
    .dw ISR_50, ISR_51, ISR_52, ISR_53, ISR_54, ISR_55, ISR_56, ISR_57
    .dw ISR_58, ISR_59, ISR_5A, ISR_5B, ISR_5C, ISR_5D, ISR_5E, ISR_5F
    .dw ISR_60, ISR_61, ISR_62, ISR_63, ISR_64, ISR_65, ISR_66, ISR_67
    .dw ISR_68, ISR_69, ISR_6A, ISR_6B, ISR_6C, ISR_6D, ISR_6E, ISR_6F
    .dw ISR_70, ISR_71, ISR_72, ISR_73, ISR_74, ISR_75, ISR_76, ISR_77
    .dw ISR_78, ISR_79, ISR_7A, ISR_7B, ISR_7C, ISR_7D, ISR_7E, ISR_7F
;
; Interrupt handler
;
ISR_00:
    ld a, 0x00
    call ECHO
    reti

ISR_01:
    ld a, 0x01
    call ECHO
    reti

ISR_02:
    ld a, 0x02
    call ECHO
    reti

ISR_03:
    ld a, 0x03
    call ECHO
    reti

ISR_04:
    ld a, 0x04
    call ECHO
    reti

ISR_05:
    ld a, 0x05
    call ECHO
    reti

ISR_06:
    ld a, 0x06
    call ECHO
    reti

ISR_07:
    ld a, 0x07
    call ECHO
    reti

ISR_08:
    ld a, 0x08
    call ECHO
    reti

ISR_09:
    ld a, 0x09
    call ECHO
    reti

ISR_0A:
    ld a, 0x0A
    call ECHO
    reti

ISR_0B:
    ld a, 0x0B
    call ECHO
    reti

ISR_0C:
    ld a, 0x0C
    call ECHO
    reti

ISR_0D:
    ld a, 0x0D
    call ECHO
    reti

ISR_0E:
    ld a, 0x0E
    call ECHO
    reti

ISR_0F:
    ld a, 0x0F
    call ECHO
    reti

ISR_10:
    ld a, 0x10
    call ECHO
    reti

ISR_11:
    ld a, 0x11
    call ECHO
    reti

ISR_12:
    ld a, 0x12
    call ECHO
    reti

ISR_13:
    ld a, 0x13
    call ECHO
    reti

ISR_14:
    ld a, 0x14
    call ECHO
    reti

ISR_15:
    ld a, 0x15
    call ECHO
    reti

ISR_16:
    ld a, 0x16
    call ECHO
    reti

ISR_17:
    ld a, 0x17
    call ECHO
    reti

ISR_18:
    ld a, 0x18
    call ECHO
    reti

ISR_19:
    ld a, 0x19
    call ECHO
    reti

ISR_1A:
    ld a, 0x1A
    call ECHO
    reti

ISR_1B:
    ld a, 0x1B
    call ECHO
    reti

ISR_1C:
    ld a, 0x1C
    call ECHO
    reti

ISR_1D:
    ld a, 0x1D
    call ECHO
    reti

ISR_1E:
    ld a, 0x1E
    call ECHO
    reti

ISR_1F:
    ld a, 0x1F
    call ECHO
    reti

ISR_20:
    ld a, 0x20
    call ECHO
    reti

ISR_21:
    ld a, 0x21
    call ECHO
    reti

ISR_22:
    ld a, 0x22
    call ECHO
    reti

ISR_23:
    ld a, 0x23
    call ECHO
    reti

ISR_24:
    ld a, 0x24
    call ECHO
    reti

ISR_25:
    ld a, 0x25
    call ECHO
    reti

ISR_26:
    ld a, 0x26
    call ECHO
    reti

ISR_27:
    ld a, 0x27
    call ECHO
    reti

ISR_28:
    ld a, 0x28
    call ECHO
    reti

ISR_29:
    ld a, 0x29
    call ECHO
    reti

ISR_2A:
    ld a, 0x2A
    call ECHO
    reti

ISR_2B:
    ld a, 0x2B
    call ECHO
    reti

ISR_2C:
    ld a, 0x2C
    call ECHO
    reti

ISR_2D:
    ld a, 0x2D
    call ECHO
    reti

ISR_2E:
    ld a, 0x2E
    call ECHO
    reti

ISR_2F:
    ld a, 0x2F
    call ECHO
    reti

ISR_30:
    ld a, 0x30
    call ECHO
    reti

ISR_31:
    ld a, 0x31
    call ECHO
    reti

ISR_32:
    ld a, 0x32
    call ECHO
    reti

ISR_33:
    ld a, 0x33
    call ECHO
    reti

ISR_34:
    ld a, 0x34
    call ECHO
    reti

ISR_35:
    ld a, 0x35
    call ECHO
    reti

ISR_36:
    ld a, 0x36
    call ECHO
    reti

ISR_37:
    ld a, 0x37
    call ECHO
    reti

ISR_38:
    ld a, 0x38
    call ECHO
    reti

ISR_39:
    ld a, 0x39
    call ECHO
    reti

ISR_3A:
    ld a, 0x3A
    call ECHO
    reti

ISR_3B:
    ld a, 0x3B
    call ECHO
    reti

ISR_3C:
    ld a, 0x3C
    call ECHO
    reti

ISR_3D:
    ld a, 0x3D
    call ECHO
    reti

ISR_3E:
    ld a, 0x3E
    call ECHO
    reti

ISR_3F:
    ld a, 0x3F
    call ECHO
    reti

ISR_40:
    ld a, 0x40
    call ECHO
    reti

ISR_41:
    ld a, 0x41
    call ECHO
    reti

ISR_42:
    ld a, 0x42
    call ECHO
    reti

ISR_43:
    ld a, 0x43
    call ECHO
    reti

ISR_44:
    ld a, 0x44
    call ECHO
    reti

ISR_45:
    ld a, 0x45
    call ECHO
    reti

ISR_46:
    ld a, 0x46
    call ECHO
    reti

ISR_47:
    ld a, 0x47
    call ECHO
    reti

ISR_48:
    ld a, 0x48
    call ECHO
    reti

ISR_49:
    ld a, 0x49
    call ECHO
    reti

ISR_4A:
    ld a, 0x4A
    call ECHO
    reti

ISR_4B:
    ld a, 0x4B
    call ECHO
    reti

ISR_4C:
    ld a, 0x4C
    call ECHO
    reti

ISR_4D:
    ld a, 0x4D
    call ECHO
    reti

ISR_4E:
    ld a, 0x4E
    call ECHO
    reti

ISR_4F:
    ld a, 0x4F
    call ECHO
    reti

ISR_50:
    ld a, 0x50
    call ECHO
    reti

ISR_51:
    ld a, 0x51
    call ECHO
    reti

ISR_52:
    ld a, 0x52
    call ECHO
    reti

ISR_53:
    ld a, 0x53
    call ECHO
    reti

ISR_54:
    ld a, 0x54
    call ECHO
    reti

ISR_55:
    ld a, 0x55
    call ECHO
    reti

ISR_56:
    ld a, 0x56
    call ECHO
    reti

ISR_57:
    ld a, 0x57
    call ECHO
    reti

ISR_58:
    ld a, 0x58
    call ECHO
    reti

ISR_59:
    ld a, 0x59
    call ECHO
    reti

ISR_5A:
    ld a, 0x5A
    call ECHO
    reti

ISR_5B:
    ld a, 0x5B
    call ECHO
    reti

ISR_5C:
    ld a, 0x5C
    call ECHO
    reti

ISR_5D:
    ld a, 0x5D
    call ECHO
    reti

ISR_5E:
    ld a, 0x5E
    call ECHO
    reti

ISR_5F:
    ld a, 0x5F
    call ECHO
    reti

ISR_60:
    ld a, 0x60
    call ECHO
    reti

ISR_61:
    ld a, 0x61
    call ECHO
    reti

ISR_62:
    ld a, 0x62
    call ECHO
    reti

ISR_63:
    ld a, 0x63
    call ECHO
    reti

ISR_64:
    ld a, 0x64
    call ECHO
    reti

ISR_65:
    ld a, 0x65
    call ECHO
    reti

ISR_66:
    ld a, 0x66
    call ECHO
    reti

ISR_67:
    ld a, 0x67
    call ECHO
    reti

ISR_68:
    ld a, 0x68
    call ECHO
    reti

ISR_69:
    ld a, 0x69
    call ECHO
    reti

ISR_6A:
    ld a, 0x6A
    call ECHO
    reti

ISR_6B:
    ld a, 0x6B
    call ECHO
    reti

ISR_6C:
    ld a, 0x6C
    call ECHO
    reti

ISR_6D:
    ld a, 0x6D
    call ECHO
    reti

ISR_6E:
    ld a, 0x6E
    call ECHO
    reti

ISR_6F:
    ld a, 0x6F
    call ECHO
    reti

ISR_70:
    ld a, 0x70
    call ECHO
    reti

ISR_71:
    ld a, 0x71
    call ECHO
    reti

ISR_72:
    ld a, 0x72
    call ECHO
    reti

ISR_73:
    ld a, 0x73
    call ECHO
    reti

ISR_74:
    ld a, 0x74
    call ECHO
    reti

ISR_75:
    ld a, 0x75
    call ECHO
    reti

ISR_76:
    ld a, 0x76
    call ECHO
    reti

ISR_77:
    ld a, 0x77
    call ECHO
    reti

ISR_78:
    ld a, 0x78
    call ECHO
    reti

ISR_79:
    ld a, 0x79
    call ECHO
    reti

ISR_7A:
    ld a, 0x7A
    call ECHO
    reti

ISR_7B:
    ld a, 0x7B
    call ECHO
    reti

ISR_7C:
    ld a, 0x7C
    call ECHO
    reti

ISR_7D:
    ld a, 0x7D
    call ECHO
    reti

ISR_7E:
    ld a, 0x7E
    call ECHO
    reti

ISR_7F:
    ld a, 0x7F
    call ECHO
    reti

;---------------------------

; RST38 handler
    .org 0x0038
    ld a, 0x38
    call ECHO
    reti

; NMI handler
    .org 0x0066
    ld a, 0x66
    call ECHO
    retn

HELLO:
    ld b, 8
    ld c, 2
    ld hl, HELLO_STR
    otir
    ret
HELLO_STR:
    .str "Hello!\r\n"
