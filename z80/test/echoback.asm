VECT_TBL    .equ    0x0100

    .area TEST (ABS)
    .org 0x0000

    di
    ld sp, 0x0000
    ld hl, TABLE
    ld a, h
    ld i, a
    im 2
    ei

    ; RX1 int 7F
    ld a, 0x55
    out (0x01), a

;    call HELLO

LOOP:
    jp LOOP

; echo back
ECHO:
    exx
ECHO_LOOP:
    in a, (0x02)        ; read a character
    out (0x02), a       ; write a character
    in a, (0x03)        ; check remaining data
    or a
    jr nz, ECHO_LOOP
    exx
    ret

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
ISR_01:
ISR_02:
ISR_03:
ISR_04:
ISR_05:
ISR_06:
ISR_07:
ISR_08:
ISR_09:
ISR_0A:
ISR_0B:
ISR_0C:
ISR_0D:
ISR_0E:
ISR_0F:
ISR_10:
ISR_11:
ISR_12:
ISR_13:
ISR_14:
ISR_15:
ISR_16:
ISR_17:
ISR_18:
ISR_19:
ISR_1A:
ISR_1B:
ISR_1C:
ISR_1D:
ISR_1E:
ISR_1F:
ISR_20:
ISR_21:
ISR_22:
ISR_23:
ISR_24:
ISR_25:
ISR_26:
ISR_27:
ISR_28:
ISR_29:
ISR_2A:
ISR_2B:
ISR_2C:
ISR_2D:
ISR_2E:
ISR_2F:
ISR_30:
ISR_31:
ISR_32:
ISR_33:
ISR_34:
ISR_35:
ISR_36:
ISR_37:
ISR_38:
ISR_39:
ISR_3A:
ISR_3B:
ISR_3C:
ISR_3D:
ISR_3E:
ISR_3F:
ISR_40:
ISR_41:
ISR_42:
ISR_43:
ISR_44:
ISR_45:
ISR_46:
ISR_47:
ISR_48:
ISR_49:
ISR_4A:
ISR_4B:
ISR_4C:
ISR_4D:
ISR_4E:
ISR_4F:
ISR_50:
ISR_51:
ISR_52:
ISR_53:
ISR_54:
ISR_55:
ISR_56:
ISR_57:
ISR_58:
ISR_59:
ISR_5A:
ISR_5B:
ISR_5C:
ISR_5D:
ISR_5E:
ISR_5F:
ISR_60:
ISR_61:
ISR_62:
ISR_63:
ISR_64:
ISR_65:
ISR_66:
ISR_67:
ISR_68:
ISR_69:
ISR_6A:
ISR_6B:
ISR_6C:
ISR_6D:
ISR_6E:
ISR_6F:
ISR_70:
ISR_71:
ISR_72:
ISR_73:
ISR_74:
ISR_75:
ISR_76:
ISR_77:
ISR_78:
ISR_79:
ISR_7A:
ISR_7B:
ISR_7C:
ISR_7D:
ISR_7E:
ISR_7F:
    call ECHO
    reti

; RST38 handler
    .org 0x0038
    call ECHO
    reti

; NMI handler
    .org 0x0066
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
