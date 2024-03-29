# CP/M DISK Imageの作成方法

## 概要
本システムでは、CP/MのディスクをmicroSD Cardでエミュレーションしている。ディスクイメージはmicroSD Card上のファイルとして管理されるため、複数のイメージファイルを置いておけば、それらは独立したディスクドライブとみなすことができる。

microSD Cardは**FAT32**でフォーマットされていること。

ディスクイメージファイルの命名規則
  - DISKnn.IMG (nnは00-15で、ドライブA:-P:に対応する)

ドライブA:であるDISK00.IMGはCP/Mのシステム(CCP+BDOS)を含む必要がある。メモリの関係上、BIOSでは５ドライブまでしかサポートしていない。したがって実際に使えるのは00-04までである。

ディスクイメージは、BIOSのDPBの形式に合致していなければならない。cpmtoolsでディスクイメージを生成する際には`diskdefs`で定義した以下のフォーマットを指定する。
```
diskdef sdcard
seclen 512
tracks 256
sectrk 64
blocksize 8192
maxdir 256
skew 0
boottrk 1
os 2.2
end
```

このフォーマットではboottrkで1トラックが予約されており、システムディスク(`IMAGE00.IMG`)では、CP/Mのシステム(CCP+BDOS)がここに格納される。


ATmega128側のファームウェアで利用しているSD Cardのアクセスライブラリは、すでに存在するファイルに対してしか書き込み操作ができない。このためイメージファイルは全データ(約8GB)が事前に生成されている必要がある。

## ディスクイメージファイルの生成方法
`z80/cpm22/image/Makefile`では以下のイメージ生成をサポートしている。

### CP/M2.2 システムディスク
[CP/M 2.2 BINARY](http://www.cpm.z80.de/download/cpm22-b.zip)
をベースに`DISK00.IMG`を生成する。
```
make
```
#### `z80/cpm22/sys/CPM.SYS`を使用する場合
[CP/M 2.2 ASM SOURCE](http://www.cpm.z80.de/download/cpm2-asm.zip)のソースコードをベースに構築した`CPM.SYS`を使用する場合、`z80/cpm22/image/Makefile`を以下のように変更する。
```
CCPBDOS   = ../sys/CPM.SYS
#CCPBDOS  = $(CPM22_DIR)/CPM.SYS
```

### 空のディスクイメージ
`EMPTY.IMG`が生成されるので、`DISKnn.IMG`にコピーして使用するか、これをベースに新たなイメージを作成する。
```
make empty
```

### ZORK I/II/III
```
make zork
```
`ZORK.IMG`が生成されるので、`DISKnn.IMG`にコピーして使用する。

### 任意のディスクイメージ
#### cpmcpを使う方法
cpmtoolsの`cpmcp`を使用し、任意のファイルをディスクイメージ上に転送することで、任意のイメージを作成できる。

例えば[CP/M software Download site](#cpm-software-download-site)などにはたくさんのCP/Mのバイナリファイルが存在する。その中から必要なファイルをピックアップしてディスクイメージを作成することもできる。

以下の例では、`./tmp`の全ファイルを空のディスクイメージに転送し、`DISK01.IMG`として`B:`ドライブに割り当てる。
```
cpmcp -f sdcard EMPTY.IMG ./tmp/*.* 0:
cp EMPTY.IMG DISK01.IMG
```

#### シェルスクリプト mkimg.shを使う方法
[../z80/cpm22/image/mkimg.sh](../z80/cpm22/image/mkimg.sh)を使うと、`EMPTY.IMG`を使わなくても直接イメージの生成が可能。
- 使い方:
  ```
  Usage: mkimg.sh <fmt> <img> [files ...]
        <fmt>: format defined in /etc/cpmtools/diskdefs
        <img>: image file name
        [files ...]: CP/M files (optional)
  ```
- 実行例:
  ```
  $ ./mkimg.sh sdcard test.img tmp/zork/*.*
  1021+0 records in
  1021+0 records out
  8364032 bytes (8.4 MB, 8.0 MiB) copied, 0.149639 s, 55.9 MB/s
  0:
  file_id.diz
  zork1.com
  zork1.dat
  zork2.com
  zork2.dat
  zork3.com
  zork3.dat
  ```

## CP/M software Download site
- [The Unofficial CP/M Web site](http://www.cpm.z80.de/)
  - [CP/M binary](http://www.cpm.z80.de/binary.html)
  - [CP/M source](http://www.cpm.z80.de/source.html)
- [Commercial CP/M Software](http://www.retroarchive.org/cpm/)
  - [ZORK I/II/III](http://www.retroarchive.org/cpm/games/zork123_80.zip)
