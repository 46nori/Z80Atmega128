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

## 2023/11/11
10/24にeBayで購入した、旧東ドイツ製 Z80互換CPU(デッドコピー)の`U880`がウクライナから届いた。
パッケージのシルクは`80A-CPU MME`となっているが、[Wikipedia](https://en.wikipedia.org/wiki/U880)によるとこれはピン間隔2.54mmの輸出版らしい。
換装して動作確認したところ、あっさり動いた。あたりまえか。

以下が[eBay](https://www.ebay.com/itm/324007899221)にあったスペックだが、Soviet cloneってのはちょっと違うんじゃないか？
```
80A CPU MME Soviet Clone Z80A IC 4MHz NMOS MCPU logic 8-bit microprocessor NEW

MME 80A-CPU

Soviet Clone Z80A
Manufacturer: MME Erfurt
Type: Microprocessor
Introduced: 198х
Name (codename): Z80A clone
Code (Copyright): 80A-CPU (no)
Specification number: no
Core speed: 4MHz
BUS: 4MHz, 8-bit
Transistors хх
Technology: 3.0-micron NMOS
L1/L2 cache: no / no
Can address: 64 kB of physical memory
V core: 5V
Power dissipation (max.): х W
Operating temperature (min./max.): 0 — 70°C
Package (Socket): Plastic DIP 40-pin (40-pin DIL)
Dimensions: 52 x 15 x 8mm
```
[こんな文書](https://datasheet.datasheetarchive.com/originals/scans/Scans-048/DSAGER000639.pdf)にUA880の記載がある。


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

## 2023/11/16
- 以下のパッチをあてるときにエラーになる場合があるので、Issueを上げた。([Issue 10](https://github.com/46nori/Z80Atmega128/issues/10), [Issue 12](https://github.com/46nori/Z80Atmega128/issues/12))
  - z80/cpm22/sys/CPM22-asz80.patch
  - z80/cpm3/image/CPM3-LDRBIOS.patch
  ```
  vscode@Z80ATmega128:/z80/cpm3/image$ make

  ...

  patch -i CPM3-LDRBIOS.patch ./tmp/cpm3on2/LDRBIOS.ASM
  patching file ./tmp/cpm3on2/LDRBIOS.ASM
  Hunk #1 FAILED at 110 (different line endings).
  1 out of 1 hunk FAILED -- saving rejects to file ./tmp/cpm3on2/LDRBIOS.ASM.rej
  make: *** [Makefile:89: tmp/cpm3on2] Error 1
  ```
  - 原因
    - checkout時にファイルの改行コードがLFになるから。パッチ適用先のCP/MのファイルはCRLFなので、Gitで勝手にLFに変換されてしまうと行が一致しないのでエラーになる。
  - 対策
    - `.gitattributes`に以下を追加して、強制的に改行コードをCRLFにする。
      ```
      *.patch text eol=crlf
      ```
    - さらにpatchコマンドにバイナリ比較オプション`--binary`を追加したら解決した。(だが、なぜこれが必要なのかよくわからない。。)
- Ubuntu 22.04上で`mkfs.cpm`でディスクイメージ作るとエラーになる。`apt install cpmtool`した`/etc/cpmtools/diskdefs`に`sdcard`の定義が存在しないのが原因。コマンドラインで指定できないのでdiskdefsに定義を追加するしかない。wontfix扱いで[Issue 13](https://github.com/46nori/Z80Atmega128/issues/13)を登録した。

## 2023/11/18
10/24に[eBay](https://www.ebay.com/itm/165487756685)で発注した`Z8400APS`(5個セット)が届いた。深圳からだけどEconomy International Shipping($2USD)だったので時間かかった。

## 2023/11/24
- [樫木総業](https://www.kashinoki.shop)に発注していたNECのuPD780C-1が届いた。
- いまのところZ80Aのコレクションはこんな感じ。
  |Manufacturer|型番|ロット?|その他|
  |---|---|---|---|
  |Zilog|Z8400APS |8320|中学生の頃入手|
  |Zilog|Z8400APS |9013|eBayで購入|
  |SHARP|LH0080A  |9640|SHARP MZ-80Bで採用|
  | NEC |uPD780C-1|8338X5|NEC PC-8001で採用|
  | MME |80A-CPU (U880)||eBayで購入。旧東ドイツ製。輸出版。|

## 2023/11/25
- Z80を何度も挿抜しているとチップもソケットも傷がつくので、評価用にゼロプレッシャーソケットの基板を1枚作った。
- eBayで購入した5個セットのZ8400APS、全数チェックしたら1個故障していた。(泣)

#### 基板の動作確認時に地味にハマったことのメモ
1. TinyMonitorの`test`コマンドがハングアップする。  
   Z80を載せる。/BUSACKによるZ80のバス解放をAVRのPD6で確認しているため。
2. `Insert microSD Card`が表示される。  
   microSD Card slotの挿入状態が正しく取れていない。以下を確認。  
   - PB6のハンダ不良
   - microSD Card slotの9番ピン、10番ピン(GND)のハンダ不良

## 2023/12/26
冬休みに入ったので、([Issue #11](https://github.com/46nori/Z80Atmega128/issues/11))に手をつけることにした。

SELDSKが遅いため、ファイルコピーなどドライブの切り替えが発生するとパフォーマンスが著しく低下する問題。ストレスフルなので何とかしたい。

Petit FatFsではファイルが同時に1つしかオープンできない制約があるため、SELDSKでDISKイメージファイルを切り替えるたびにpf_open()を実行する必要がある。ここに2,3秒かかっている。すべてのDISKイメージファイルがあらかじめオープンしておけるなら、pf_open()が不要になるので相当早くなるはずだ。

複数ファイルの同時オープンは[FatFs](http://elm-chan.org/fsw/ff/)を使用すれば可能。そもそもPetit FatFsを採用したのは以下の理由なので、フットプリントの懸念がなければFatFsに乗り換えられる。
- CP/Mでは同時に複数のディスク(イメージファイル)にアクセスする必要がない。(が、まさかpf_open()がこんなに遅いとは思わなかった。)
- AVRのRAMが4KBしかない。当初は全体でどのくらいメモリを食うか正確に見積もるのは難しかったため、フットプリントはなるべく小さくしたかった。

改めて[FatFsのフットプリント](http://elm-chan.org/fsw/ff/doc/appnote.html#memory)を調べてみた。
- 条件
  - $V=1$ : 1物理ドライブ
  - $F=5$ : 5ファイル (A: - E:の5 DISKイメージ分)
- 必要なSRAMサイズ
  - .bss = $V \times 4 + 2$ = 6
  - `FF_FS_TINY	== 0`の場合
    - .bss + Work area $= 6 + V \times 560 + F \times 546 = 3296$
  - `FF_FS_TINY	== 1`の場合
    - .bss + Work area $= 6 + V \times 560 + F \times 34 = 736$

現在の残SRAMサイズは2048bytesなので、**`FF_FS_TINY == 1`なら問題ない。**

ということで、ブランチ`feature-FatFs`で作業することにした。
- FatFsのソース管理用に`avr/src/fatfs/`を作成。(`avr/src/petitfs/`の代替)
- FatFs本体(R0.15)とパッチを入手し、patch3まで適用してコミット。
  - [ff15.zip](http://elm-chan.org/fsw/ff/arc/ff15.zip) : FatFs本体(R0.15)
  - [Patches](http://elm-chan.org/fsw/ff/patches.html) : パッチ
  - `ffsystem.c`と`ffunicode.c`は不要
- サンプルをダウンロードしてAVR用のソースを取り出す。
  - [ffsample.zip](http://elm-chan.org/fsw/ff/ffsample.zip) : サンプル
  - `diskio.h`,`mmc_avr.h`,`mmc_avr_spi.c`をカスタマイズして使用する。
  他は不要。

## 2023/12/27
Petit FatFsからFatFsへの乗り換え作業を実施。意外と簡単に移行できた。  
非常に速くなった！特にPIPのファイルコピーが劇的に改善された。  
現在のメモリ残量は1265bytes。


### 変更点
- Microchip StudioのSolution Explorerで、`petitfs/`を削除、`fatfs/`を追加。
- `petitfs/`は削除。
- `ffconf.h`
  - ディレクトリ関連のAPIは不要。RTCはハード的に存在しないので不要。LFNも使用しない。文字列処理やUNICODE対応も不要。
  - 以下のようにコンフィグレーションを変更する。
    ```
    #define FF_FS_MINIMIZE  2
    #define FF_FS_TINY      1
    #define FF_FS_NORTC     1
    ```
- `diskio.c`
  - MMCの実装以外は削除。
- `diskio.h`
  - `mmc_avr_spi.c`に合うように、サンプルから中身をほぼコピー。
- `mmc_avr_spi.c`
  - Platform依存のSPIやポートのマクロ定義を`petitfs/diskio_avr.c`の実装からコピぺ。
  - `power_on()`に、SPIの初期化コードを`petitfs/diskio_avr.c`の実装からコピぺ。
  - タイムアウト処理を行っているので`disk_timerproc()`を10msごとに呼び出す必要がある。isr.cのTIMER2の10msの周期割り込みハンドラ内で呼び出す。
- `em_diskio.c`
  - FatFSのAPIに乗り換え。
  - `init_em_diskio()`でDISKイメージファイル5つをすべてオープン。
  - `OUT_0A_DSK_SelectDisk()`でのオープン処理を削除し、カレントDISKイメージのポインタだけ切り替えるように変更。
  - Writeのフラッシュ処理を`f_sync()`に変更。

