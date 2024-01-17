# CP/M 3 on Z80ATmega128 Board
## Overview
It is able to make CP/M 3 work on CP/M 2.2 system.
However, due to its limited 64KB of memory, it works as the non-bank memory version of CP/M 3. This document provides an explanation of its operation, instructions on generating the required files, and guidance on initiating the system.

### How it works
It is assumed that the image of the system disk (A:) is equivalent to CP/M2.2, and that the following files exist in the directory.
- `CPM3.SYS` : CP/M 3 BDOS and BIOS
- `CPMLDR.COM` : CPM3.SYS Loader
- `CCP.COM` : CCP as a Transient command
```
        +------------+
Track 0 | CPM2.SYS   |
        |------------|
Track 1 |    ...     |
        | CPMLDR.COM |
  ...   | CPM3.SYS   |
        | CCP.COM    |
        |    ...     |
        +------------+
```
Initiation steps for CP/M 3
1. Boot CP/M2.2 built according with `z80/cpm2`.
2. Execute `CPMLDR.COM` from the command line.
3. `CPM3.SYS` is loaded and executed. (From here, CP/M 3 is working.)
4. `CCP.COM` is loaded and executed.

`CPMLDR.COM` and `CPM3.SYS` are generated on the CP/M 2.2 system. (The procedure is explained later.)  
The memory map is configured as shown below.
### Memory Map
```
       CP/M 2.2           CP/M 3
0000H +--------+         +--------+
0100H |--------|         |--------|
      |  TPA   |         |  TPA   |  
      | (55K)  |         | (52K)  |
      |        |   D000H |--------|
DC00H |--------|         |        |
      |  CCP   |         | BDOS3  |
      | (800H) |         | (1D00H)|
E400H |--------|         |        |
      | BODS2  |   ED00H |--------|
      | (800H) |         | BIOS3  |
      |        |         | (400H) |
F200H |--------| - - - - |--------|
      | BIOS2  |         | BIOS2  |
      | (E00H) |         | (E00H) |
FFFFH +--------+ - - - - +--------+
```
BIOS3 reuses BIOS2 of CP/M 2.2 as is.


## How to create a DISK image
1. Execute `make` on `z80/cpm3/image`.
     `DISK00.IMG` will be generated. This system disk for CP/M 2.2 includes the files necessary to generate CP/M 3 and the CP/M 3 transient commands. (However, please note that SUBMIT.COM is of CP/M 2.2's, not CP/M 3's.)
2. Copy `DISK00.IMG` to the microSD Card and boot the system.
3. CP/M 2.2 starts up.
4. Execute batch file `GEN.SUB` so that  `CPMLDR.COM` and `CP/M3.SYS` are generated.
    You will be prompted to enter parameters on `GENCPM.COM`, so enter them as follows.(See [here](#gencpmcomによるcpm3sysの構成) in details of parametrs)
    ```
    A>submit gen

    Reload CCP+BDOS.

    A>RMAC LDRBIOS
    CP/M RMAC ASSEM 1.1
    0084
    006H USE FACTOR
    END OF ASSEMBLY

    Reload CCP+BDOS.

    A>LINK CPMLDR,SCB,LDRBIOS
    LINK 1.3

    @CIVEC   FE22   @COVEC   FE24   @AIVEC   FE26   @AOVEC   FE28
    @LOVEC   FE2A   @BNKBF   FE35   @CRDMA   FE3C   @CRDSK   FE3E
    @FX      FE43   @RESEL   FE41   @VINFO   FE3F   @USRCD   FE44
    @ERMDE   FE4B   @DATE    FE58   @HOUR    FE5A   @MIN     FE5B
    @SEC     FE5C   @MXTPA   FE62

    ABSOLUTE     0000
    CODE SIZE    0BEA (0100-0CE9)
    DATA SIZE    0084 (0CEA-0D6D)
    COMMON SIZE  0000
    USE FACTOR     23

    Reload CCP+BDOS.

    A>RMAC GBIOS
    CP/M RMAC ASSEM 1.1
    0144
    006H USE FACTOR
    END OF ASSEMBLY

    Reload CCP+BDOS.

    A>LINK BIOS3[OS]=GBIOS,SCB
    LINK 1.3

    @CIVEC   FE22   @COVEC   FE24   @AOVEC   FE28   @LOVEC   FE2A
    @BNKBF   FE35   @CRDMA   FE3C   @CRDSK   FE3E   @FX      FE43
    @RESEL   FE41   @VINFO   FE3F   @USRCD   FE44   @ERMDE   FE4B
    @DATE    FE58   @HOUR    FE5A   @MIN     FE5B   @SEC     FE5C
    @MXTPA   FE62   @AIVEC   FE26

    ABSOLUTE     0000
    CODE SIZE    01D3 (0000-01D2)
    DATA SIZE    0144 (01D3-0316)
    COMMON SIZE  0000
    USE FACTOR     05

    Reload CCP+BDOS.

    A>GENCPM


    CP/M 3.0 System Generation
    Copyright (C) 1982, Digital Research

    Default entries are shown in (parens).
    Default base is Hex, precede entry with # for decimal

    Create a new GENCPM.DAT file (N) ?

    Display Load Map at Cold Boot (Y) ?

    Number of console columns (#80) ?
    Number of lines in console page (#24) ?
    Backspace echoes erased character (N) ?
    Rubout echoes erased character (Y) ?

    Initial default drive (A:) ?

    Top page of memory (FF) ? F0
    Bank switched memory (Y) ? N

    Double allocation vectors (Y) ?

    Accept new system definition (Y) ?

    BIOS3    SPR  ED00H  0400H
    BDOS3    SPR  CE00H  1F00H

    *** CP/M 3.0 SYSTEM GENERATION DONE ***
    ```

## How to launch CP/M 3
```
A>cpmldr

CP/M V3.0 Loader
Copyright (C) 1982, Digital Research

 BIOS3    SPR  EE00  0400
 BDOS3    SPR  D100  1D00

 52K TPA

A>
```

-----
## Technical detail
The generation of `CPMLDR.COM` and `CPM3.SYS` is automated by Makefile, but we will explain the details here.

### How to generate CPMLDR.COM
* Modify LDRBIOS.ASM   
  Modify `bios` to have the configuration of [Memory Map](#memory-map). (Original value is `0e900h`)
   `CPMLDR.COM` is used only to load `CPM3.SYS`, so `drives` can be left at 2.
  ```
  drives	equ	2		;number of drives supported
  bios	equ	0f200h		;address of your bios
  ```

* Generate LDRBIOS.REL  
   ```
  C>rmac ldrbios.asm
  CP/M RMAC ASSEM 1.1
  0084
  006H USE FACTOR
  END OF ASSEMBLY
  ```

* Generate CPMLDR.COM    
  Generate `CPMLDR.COM` from `LDRBIOS.REL`.  
  `CPMLDR.REL` and `SCB.REL` are provided with CP/M 3.
    ```
    C>link cpmldr,scb,ldrbios
    LINK 1.3

    @CIVEC   FE22   @COVEC   FE24   @AIVEC   FE26   @AOVEC   FE28
    @LOVEC   FE2A   @BNKBF   FE35   @CRDMA   FE3C   @CRDSK   FE3E
    @FX      FE43   @RESEL   FE41   @VINFO   FE3F   @USRCD   FE44
    @ERMDE   FE4B   @DATE    FE58   @HOUR    FE5A   @MIN     FE5B
    @SEC     FE5C   @MXTPA   FE62

    ABSOLUTE     0000
    CODE SIZE    0BEA (0100-0CE9)
    DATA SIZE    0084 (0CEA-0D6D)
    COMMON SIZE  0000
    USE FACTOR     23
    ```

### How to generate CPM3.SYS
Generate using `GENCPM.COM`. The following files are required.
- `BDOS3.SPR` : Provided by CP/M 3
- `BIOS3.REL` : Generated in this step

#### Generate `BIOS3.REL`
1. Modify GBIOS.ASM  
  Modify the file for reusing 62K CP/M 2.2 BIOS as is. The start address of BIOS2 is `F200H`. (Refer to [Memory Map](#memory-map))  
  5 drives (A:-E:) are supported.  
  Since the settings happen to be the same, there should be no need to modify them, but if they are different, set them as follows.
    ```
    drives	equ	5		;number of drives supported
    bios	equ	0f200h		;address of your bios
    ```
2. Assembling  
    ```
    C>rmac gbios
    CP/M RMAC ASSEM 1.1
    0144
    006H USE FACTOR
    END OF ASSEMBLY
    ```
3. Linking  
   `SCB.REL` is provided by CP/M 3.    
   ```
    C>link bios3[os]=gbios,scb
    LINK 1.3

    @CIVEC   FE22   @COVEC   FE24   @AOVEC   FE28   @LOVEC   FE2A
    @BNKBF   FE35   @CRDMA   FE3C   @CRDSK   FE3E   @FX      FE43
    @RESEL   FE41   @VINFO   FE3F   @USRCD   FE44   @ERMDE   FE4B
    @DATE    FE58   @HOUR    FE5A   @MIN     FE5B   @SEC     FE5C
    @MXTPA   FE62   @AIVEC   FE26

    ABSOLUTE     0000
    CODE SIZE    01D3 (0000-01D2)
    DATA SIZE    0144 (01D3-0316)
    COMMON SIZE  0000
    USE FACTOR     05
    ```

#### Generate CPM3.SYS using GENCPM.COM
Set the parameters of `GENCPM.COM` as follows to match the [Memory Map](#memory-map).
The top page of memory can only be specified in units of 512 bytes.
```
C>gencpm


CP/M 3.0 System Generation
Copyright (C) 1982, Digital Research

Default entries are shown in (parens).
Default base is Hex, precede entry with # for decimal

Create a new GENCPM.DAT file (N) ?         Preserve settings?

Display Load Map at Cold Boot (Y) ?

Number of console columns (#80) ?          Console settings
Number of lines in console page (#24) ?
Backspace echoes erased character (N) ?
Rubout echoes erased character (Y) ?

Initial default drive (A:) ?               

Top page of memory (FF) ? F0               Memory allocation (512byte units)
Bank switched memory (Y) ? N               Non bank memory

Accept new system definition (Y) ?

 BIOS3    SPR  ED00H  0400H
 BDOS3    SPR  D000H  1D00H

*** CP/M 3.0 SYSTEM GENERATION DONE ***
```
