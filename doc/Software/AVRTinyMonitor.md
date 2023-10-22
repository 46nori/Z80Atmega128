# ATmega128 Tiny Monitor

The ATmega Tiny Monitor has the following main functions:
- Control of AVR microprocessor
- Access to external SRAM shared with Z80
- Debugging functions for the Z80

As a typical use case, the following commands can be used to download and setup CP/M
- Write CP/M BIOS to EEPROM. (`esave2`)
- Download and boot the INTEL HEX image (`cpm22/sys/cpm.ihx`) of CP/M. (`xload`, `reset`)

## Command Reference
### Legend
`<>` is mandatory parameter.  
`[]` is optional parameter.


## Memory command
### Memory dump
`[adr]` and `[len]` are optional. Default values are 0 and 16 respectively.  
`[adr]` is automatically incremented.
|command| Parameters      | Memory        |
|-------|-----------------|---------------|
| `d`   | `[adr]` `[len]` | External SRAM |
| `di`  | `[adr]` `[len]` | Internal SRAM |
| `df`  | `[adr]` `[len]` | Flash ROM     |
| `de`  | `[adr]` `[len]` | EEPROM        |

### Byte READ and WRITE
`[adr]` is optional. Default values is 0. It is automatically incremented.
|command|Parameters| Memory        | Operation |
|-------|----------|---------------|:---------:|
| `r`   | `[adr]`  | External SRAM | READ      |
| `w`   | `[adr]`  | External SRAM | WRITE     |
| `ri`  | `[adr]`  | Internal SRAM | READ      |
| `wi`  | `[adr]`  | Internal SRAM | WRITE     |

### Misc.
|command | Parameters              | Memory        | Operation |
|--------|-------------------------|---------------|-----------|
| `f`    | `<dat>` `<adr>` `<len>` | External SRAM | Fills `<dat>` into the external memory for length:`<len>` from the address:`<adr>`. |
| `test` | `[adr]`                 | External SRAM | Fill in the memory with test data from specified address:`[adr]` and verify the checksum.<br>The default address is 0x2000 if `[adr]` is omitted.<br>It is helpful for quick checks of external memory access. |
| `mem`  |                         | Internal SRAM | Show remaining memory size. Note that it includes the stack area. |

## Data transfer command
- Use `esave2` to save the BIOS to EEPROM for automatic CP/M startup.
- Use `xload` to download CP/M image (`z80/cpm22/sys/cpm.ihx`).

| command  | Parameters       | From          |  To             | Operation                   |
|----------|------------------|---------------|-----------------|-----------------------------|
| `lx`     | `<adr>`          | XMODEM Sender | Internal SRAM   | Load binary image by XMODEM |
| `sx`     | `<adr>` `<len>`  | Internal SRAM | XMODEM Reciever | Save binary image by XMODEM |
| `bload`  | `<adr>`          | XMODEM Sender | External SRAM   | Load binary image by XMODEM |
| `xload`  |                  | XMODEM Sender | External SRAM   | Load INTEL HEX image by XMODEM<br>The binary is located by being converted from the INTEL HEX format. |
| `eload`  |`<dst>` `<src>` `<len>` | EEPROM  | External SRAM   | Load EEPROM data from address:`<src>` for the length:`<len>` into the external SRAM from the address:`<dst>`. | 
| `esave`  |`<dst>` `<src>` `<len>` | External SRAM | EEPROM    | Save External SRAM data from the address:`<src>` for the length:`<len>` to the EEPROM from the address:`<dst>`. |
| `esave2` |`<dst>` `<src>` `<len>` | External SRAM | EEPROM    | `<src>` and `<len>` are stored into EEPROM at address:`<dst>`.<br>And save External SRAM data from the address:`<src>` for the length:`<len>` to the EEPROM from the address:`<dst>`+4. |

## AVR Intterrupt control command
| command | Parameters | Opetarion        |
|---------|------------|------------------|
| `cli`   |            | Disable intrrupt |
| `sti`   |            | Enable  intrrupt |

## Z80 command
### Z80 Control
| command | Parameters | Opetarion        |
|---------|------------|------------------|
| `reset` | `[0]`      | Reset Z80. If `0` is specified, HALT instruction(0x76) will be set to 0x0000 and stay in the HALT state. |
| `nmi`   |            | Assert Z80 /NMI. |
| `int`   | `<dat>`    | Assert Z80 /INT. Interrupt the Z80. Depending on the Z80's interrupt mode, <dat> is interpreted as follows:<br>Mode0: Instruction<br>Mode1: Ignored<br>Mode2: Interrupt vector number |
| `sts`   |            | Show the signal state of /BUSREQ, /BUSACK and /HALT. |

### Z80 Debugger
| command | Parameters | Opetarion                     |
|---------|------------|-------------------------------|
| `brk`   | `[adr]`    | Set breakpoint. List breakpoints if `[adr]` is omitted. |
| `del`   | `<adr>`    | Delete breakpoint.           |
| `cont`  |            | Continue program after break. |
- Example
    ```
    >brk $f3dd
    >brk
    0: $f3dd $c3 
    ```
    Z80 registers and stack data are shown when hit the breakpoint.
    ```
    >>>>Break! at $f3dd
    AF=$0044 [Z:1 C:0] I=$f3
    BC=$0000 DE=$dc06 HL=$f233
    IX=$efff IY=$fd77 SP=$dbfe
    (SP+0)=$f3dd
    (SP+2)=$5cc3
    (SP+4)=$c3df
    ```

## Help command
| command | Parameters | Opetarion           |
|---------|------------|---------------------|
| `h`     |            | Shows help message. |
- Example
    ```
    ATmega128 Tiny Monitor
    >h 
    <> : mandatory
    [] : optional
    $  : Prefix of hexadecimal
    h               : Help
    == AVR Commands ==
    r  [adr]        : Read  an ExtRAM byte
    ri [adr]        : Read  an IntRAM byte
    w  <adr> <dat>  : Write an ExtRAM byte
    wi <adr> <dat>  : Write an IntRAM byte
    f  <dat> <adr> <len> : Fill ExtRAM with <dat>
    d  [adr] [len]  : Dump ExtRAM
    di [adr] [len]  : Dump IntRAM
    df [adr] [len]  : Dump FlashROM
    de [adr] [len]  : Dump EEPROM
    lx <adr>        : Load binary to   IntRAM by XMODEM
    sx <adr> <len>  : Save binary from IntRAM by XMODEM
    bload <adr>     : Load binary    to ExtRAM by XMODEM
    xload           : Load INTEL HEX to ExtRAM by XMODEM
    eload <dst> <src> <len>  : Load EEPROM to ExtRAM
    esave  <dst> <src> <len> : Save ExtRAM to EEPROM
    esave2 <dst> <src> <len> : esave after writing <src> <len>
    mem             : Remaining IntRAM size
    sei             : Set  interrupt
    cli             : Clear interrupt
    test [adr]      : Test XMEM R/W
    == Z80 Control Commands ==
    reset [0]       : Reset, HALT if 0
    nmi             : Invoke NMI
    int <dat>       : Invoke interrupt
    sts             : Show Z80 status
    == Z80 Debug Commands ==
    brk [adr]       : Set breakpoint, show list if no adr
    del <adr>       : Delete breakpoint
    cont            : Continue from breakpoint
    ```
