## [以前の日記](Diary2.md)

## 2023/10/24
- PCB Rev2.00の更新。

## 2023/10/26
- PCB Rev2.00をPCBWayに発注。

## 2023/11/01
- PCB Rev2.00入荷。

## 2023/11/02
- Rev1.00のバグ修正したfeature-PCB2ブランチをdevelopにマージした。
  - PCB Rev2.00にパーツをマウントし動作確認できた。
  - 配線、パーツ間のクリアランスも問題なし。
  - 残念ながらUSB-Cシリアル変換モジュールの信号名のシルクが間違っている。修正忘れ。
  - [Hardware Release Note](./Hardware/HW-ReleaseNote-ja.md)などのドキュメントも更新。

## 2023/11/03
- CP/Mのディスクイメージを作りやすくするとともに、ドキュメントを追加した。
  - システムディスク、空ディスク、ZORK I/II/IIIのイメージ作成をMakefileのターゲット指定でできるようにした。
  - developブランチにマージ済み。

## 2023/11/04
- CP/M BIOSのドキュメント追加。
- Boot sequenceのドキュメント追加。

## 2023/11/05
- `z80/cpm22/image/mkimg.sh`で引数チェックのバグを修正。DiskImage-CPM22.mdも更新。
- v2.0をリリースした。

## 2023/11/12
- CP/M Plusの対応を始めた。
- [これ](http://www.cpm.z80.de/download/cpm3on2.zip)を利用すれば、CP/M 2.2からCP/M 3が起動できるみたいだ。まずCP/M 2.2を起動し、二段ロケット方式でCP/M3を起動する方式。
- 62K CP/M 2.2 BIOSがそのまま再利用できる。BIOSのジャンプテーブルを参照可能なシンボルファイルをリンクすることで、CPM3.SYSとCPMLDR.COMを生成する。
- このイメージになるように、LDRBIOS.ASMの`bios`を値を、BIOS2の先頭アドレス`f200h`に変更する。
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
- `gencpm.com`でCPM3.SYSを生成するが、このメモリマップになるようにするには、以下のようにパラメータをセットする。ノンバンクメモリバージョンで生成。
  ```
  Top page of memory (FF) ? F0
  Bank switched memory (Y) ? N
  ```

## 2023/11/13
- CP/M 3が動いた。
- BIOSとCPMLDR.COMの生成は、`gen.sub`のバッチファイルを使うのが便利だが、A:ドライブに置かないといけない。また、CP/M2.2用のsubmit.comを使う必要がある。
- A:に、CPMLDR.COM, CPM3.SYS, CCP.COMをコピーして、CPMLDR.COMを起動すると、CP/M3が立ち上がった。なんかリターンがたくさん入るけど。。
  ```
  62K CP/M-80 Ver2.2 on Z80ATmega128
  BIOS Copyright (C) 2023 by 46nori

  A>cpmldr
























  CP/M V3.0 Loader
  Copyright (C) 1998, Caldera Inc.

  BIOS3    SPR  ED00  0400
  BDOS3    SPR  CE00  1F00

  51K TPA
  A>
  ```
- PC-8801mkIIでCP/M 2.2を使っていた頃は、CP/M Plusに憧れがあったのだけど、いまのところありがたみも凄みも感じられないなぁ。。
- DIRコマンドとか？
  ```
  A>dir.com

  Scanning Directory...

  Sorting  Directory...

  Directory For Drive A:  User  0

      Name     Bytes   Recs   Attributes      Name     Bytes   Recs   Attributes
  ------------ ------ ------ ------------ ------------ ------ ------ ------------
  BDOS3    SPR    16k     77 Dir RW       BIOS3    SPR     8k      9 Dir RW
  BIOS3    SYM     8k      2 Dir RW       BNKBDOS3 SPR    16k    106 Dir RW
  CCP      COM     8k     25 Dir RW       COPYSYS  COM     8k     15 Dir RW
  CPM3     SYS    16k     72 Dir RW       CPMLDR   COM     8k     25 Dir RW
  CPMLDR   REL     8k     23 Dir RW       CPMLDR   SYM     8k      2 Dir RW
  DATE     COM     8k     26 Dir RW       DEVICE   COM     8k     57 Dir RW
  DIR      COM    16k    114 Dir RW       DUMP     COM     8k      8 Dir RW
  ED       COM    16k     73 Dir RW       ERASE    COM     8k     30 Dir RW
  GBIOS    ASM    16k    103 Dir RW       GBIOS    PRN    24k    164 Dir RW
  GBIOS    REL     8k      6 Dir RW       GBIOS    SYM     8k      9 Dir RW
  GEN      SUB     8k      1 Dir RW       GENCOM   COM    16k    115 Dir RW
  GENCPM   COM    24k    166 Dir RW       GET      COM     8k     51 Dir RW
  HELP     COM     8k     55 Dir RW       HELP     HLP    64k    496 Dir RW
  HEXCOM   CPM     8k      9 Dir RW       LDRBIOS  ASM    16k    103 Dir RW
  LDRBIOS  PRN    24k    164 Dir RW       LDRBIOS  REL     8k      6 Dir RW
  LDRBIOS  SYM     8k      9 Dir RW       LINK     COM    16k    122 Dir RW
  PATCH    COM     8k     19 Dir RW       PIP      COM    16k     68 Dir RW
  PUT      COM     8k     55 Dir RW       README   1ST     8k      9 Dir RW
  Press RETURN to Continue

  Directory For Drive A:  User  0

      Name     Bytes   Recs   Attributes      Name     Bytes   Recs   Attributes
  ------------ ------ ------ ------------ ------------ ------ ------ ------------
  RENAME   COM     8k     23 Dir RW       RESBDOS3 SPR     8k     16 Dir RW
  RMAC     COM    16k    106 Dir RW       SAVE     COM     8k     14 Dir RW
  SCB      REL     8k      3 Dir RW       SET      COM    16k     81 Dir RW
  SETDEF   COM     8k     34 Dir RW       SHOW     COM    16k     66 Dir RW
  SUBMIT   COM     8k     10 Dir RW       TYPE     COM     8k     24 Dir RW

  Total Bytes     =    576k  Total Records =    2771  Files Found =   46
  Total 1k Blocks =    370   Used/Max Dir Entries For Drive A:   46/ 256

  ```
