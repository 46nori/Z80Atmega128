# CP/M 3 on ATmega128 Board
## 概要
CP/M 3はCP/M 2.2から起動させることができる。  
ただしZ80ATmega128 Boardは64KBのメモリしかないため、Non bank memory版CP/M 3として動作する。このドキュメントでは、その仕組みと必要なファイルの生成方法、および起動方法について説明する。

### 起動の仕組み
システムディスク(A:)のイメージはCP/M2.2と同等で、ディレクトリには以下のファイルが存在することを前提としている。
- CPM3.SYS : CP/M 3のBDOSとBIOS
- CPMLDR.COM : CPM3.SYSのLOADER
- CCP.COM : トランジェントコマンド化されたCP/M 3のCCP
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
起動は以下の手順
1. `z80/cpm2`で構築したCP/M2.2でシステム起動。
2. コマンドラインから`CPMLDR.COM`を実行。
3. `CPM3.SYS`がロード・実行される。ここからCP/M 3の世界に移行。
4. `CCP.COM`がロード・実行される。


`CPMLDR.COM`と`CPM3.SYS`は、CP/M 2.2上でアセンブル、リンクして生成する。(生成方法は後述。)  
以下のメモリマップになるように構成されている。
### メモリマップ
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
BIOS3は、CP/M 2.2のBIOS2をそのまま使用する。


## イメージの作成方法
1. `z80/cpm3/image`で`make`を実行。  
    `DISK00.IMG`が生成される。CP/M2.2の起動DISKになっており、CP/M 3を生成するのに必要なファイル群と、CP/M 3のトランジェントコマンドが含まれる。(ただしSUBMIT.COMは2.2のものなので注意)
2. `DISK00.IMG`をmicroSD Cardにコピーしシステムを起動する。
3. CP/M 2.2が立ち上がる。
4. バッチファイル`GEN.SUB`を実行すると、`CPMLDR.COM`と`CPM3.SYS`が生成される。  
   `GENCPM.COM`でパラメータ入力を促されるので以下の通り入力。(パラメータの詳細は[こちら](#gencpmcomによるcpm3sysの構成))
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

## CP/M 3の起動方法
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
## 詳細
`CPMLDR.COM`と`CPM3.SYS`の生成は、Makefileにより自動化されているが、ここではその詳細を解説する。

### CPMLDR.COMの生成
* LDRBIOS.ASM の変更  
  [メモリマップ](#メモリマップ)の構成になるよう`bios`のみ変更する。(元の値は`0e900h`)  
  `CPMLDR.COM`は`CPM3.SYS`をロードするためだけに使われるので、`drives`は2のままで構わない。
  ```
  drives	equ	2		;number of drives supported
  bios	equ	0f200h		;address of your bios
  ```

* LDRBIOS.RELの生成
   ```
  C>rmac ldrbios.asm
  CP/M RMAC ASSEM 1.1
  0084
  006H USE FACTOR
  END OF ASSEMBLY
  ```

* CPMLDR.COMの生成  
  `LDRBIOS.REL`から`CPMLDR.COM`を生成する。  
  `CPMLDR.REL`と`SCB.REL`はCP/M 3で提供される。
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

### CPM3.SYSの生成
`GENCPM.COM`を使用して生成する。以下のファイルが必要。
- `BDOS3.SPR` : CP/M 3で提供される
- `BIOS3.REL` : このステップで生成する

#### `BIOS3.REL`の生成
1. GBIOS.ASMの修正  
  62K CP/M 2.2のBIOSをそのまま使用する。BIOS2の先頭アドレスは`F200H`である。([メモリマップ](#メモリマップ)参照)  
  また5台のドライブ(A:-E:)をサポートしている。  
  たまたま同じ設定なので修正不要のはずだが、もしも異なっている場合は以下のように設定する。
    ```
    drives	equ	5		;number of drives supported
    bios	equ	0f200h		;address of your bios
    ```
2. アセンブル
    ```
    C>rmac gbios
    CP/M RMAC ASSEM 1.1
    0144
    006H USE FACTOR
    END OF ASSEMBLY
    ```
3. リンク  
  `SCB.REL`はCP/M 3で提供されるものをそのまま使用する。
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

#### GENCPM.COMによるCPM3.SYSの構成
[メモリマップ](#メモリマップ)に合うように、`GENCPM.COM`のパラメータを以下のように設定する。  
Top page of memoryは512byte単位でしか指定できない。
```
C>gencpm


CP/M 3.0 System Generation
Copyright (C) 1982, Digital Research

Default entries are shown in (parens).
Default base is Hex, precede entry with # for decimal

Create a new GENCPM.DAT file (N) ?         設定を保存するか?

Display Load Map at Cold Boot (Y) ?

Number of console columns (#80) ?          コンソールの設定
Number of lines in console page (#24) ?
Backspace echoes erased character (N) ?
Rubout echoes erased character (Y) ?

Initial default drive (A:) ?               デフォルトドライブ

Top page of memory (FF) ? F0               メモリアロケーション(512byte単位)
Bank switched memory (Y) ? N               Non bank memory

Accept new system definition (Y) ?

 BIOS3    SPR  ED00H  0400H
 BDOS3    SPR  D000H  1D00H

*** CP/M 3.0 SYSTEM GENERATION DONE ***
```
