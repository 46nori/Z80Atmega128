# Z80ATmega128 Board Setup Guide
Z80Atmega128 Boardにファームウェアを焼き込み、CP/M2.2を単体動作させるまでの手順を以下に示す。

## 基板実装図
![](Fig/ComponentMountingDiagram.jpeg)

| No.|                   |                           |
|:--:|-------------------|---------------------------|
|(1) | DC Jack           |+5V 外部電源(ACアダプタ用)    |
|(2) | DC Terminal       |+5V 外部電源ターミナル        |
|(3) | DC Select         |(1)/(2)切替用ジャンパーピン   |
|(4) | POWER SW          |外部電源SW                  |
|(5) | POWER LED         |+5V通電時に点灯              |
|(6) | RESET             |システムリセット              |
|(7) | DIP SW            |[詳細](#dip-sw)|
|(8) | Disable Z80       |Z80強制停止用ジャンパーピン<br>/BUSREQ=Lにする|
|(9) | /HALT BUSACK LED  |Z80 /HALT=L, /BUSACK=H で点灯|
|(10)| LED 1 / 2 / 3     |[詳細](#led)|
|(11)| UART1 for Z80     |Z80用USB-Cシリアルインターフェース |
|(12)| UART0 for AVR     |AVR用USB-Cシリアルインターフェース |
|(13)| microSD Card slot |microSDカードスロット            |
|(14)| SD Access LED     |microSDカードアクセス時に点灯     |
|(15)| AVR ISP Connector |AVRISP mkII接続用コネクタ       |

### 電源
- 300mA以上の電流供給能力が必要。
- 5.5V以上の電圧は印加しないこと。過電圧保護は行なっていない。
- DCジャック、外部電源ターミナル、UART用USB-Cから給電できる。

### DIP SW
|  #  | 機能           |   ON   |   OFF   |
|:---:|----------------|--------|---------|
|  1  | CP/M mode      | Enable | Disable |
|  2  | UART baud rate |  19200 |    9600 |
|  3  | Not used       |        |         |
|  4  | SRAM Wait      | 1 wait |  2 wait |
- SW 1: CP/M BIOSがAVRのEEPROMに書き込まれている場合、ONにすることでCP/Mが起動する。
- SW 2: 接続する端末の通信速度にあわせる。
- SW 4: 外部SRAM(HM62256)のアクセスタイムが100nsよりも速い場合、ONにすることで1ウェイト動作になる。

### LED
|  #  | 色 | 状態                           |
|:---:|----|-------------------------------|
|  1  | 青 | microSDのopen中または失敗時に点灯 |
|  2  | 黄 | 未使用                         |
|  3  | 赤 | AVRのハートビート表示(2Hzで点滅)  |

### 注意
- PCB Rev1.00　配線にバグがあるため、ハード改修の必要がある。
   - [表面](./Hardware/PCB/PCB1.0-FP-Errata.pdf) : パターンカット7箇所
   - [裏面](./Hardware/PCB/PCB1.0-BP-Errata.pdf) : パターンカット1箇所、パッチ配線6箇所
- PCB Rev2.00　U18, U19のUARTのシルクが間違っている。

## 1. 事前準備
- Windowsに[Microchip Studio](https://www.microchip.com/en-us/tools-resources/develop/microchip-studio)をインストールする。
  - Ver7.0.2594で動作確認
- [AVRISP mkII](https://www.microchip.com/en-us/development-tool/ATAVRISP2)を使えるようにしておく。
  - 純正品は製造中止なので、互換品でもかまわない。
  - ATmega128のFUSE設定とファームウェアの書き込みに使用する。
- シリアルインターフェースで送受信ができる端末(できれば2チャンネル分)を使えるようにしておく。
  - プロトコル: 9600 or 19200 baud, 8bit / non parity / 1stop bitの調歩同期通信
  - XMODEM: 128byte check sum 送信が行えること。
  - 端末ソフト: [TeraTerm](https://ttssh2.osdn.jp/index.html.en) (Windows)など
  - 通信速度はDIP SWの設定に合わせておく。

## 2. ファームウェアの書き込み
Z80ATmega128 Boardにファームウェアを焼き込み、モニタプログラムを動作させるまでの手順を説明する。  

### 2-1. AVRファームウェアのビルド
1. WindowsでMicrochip Studioを起動。
2. `File > Open > Project/Solution...` で、`avr/avr.atsln` を選択。
3. `Build > Build Solution` を実行。
4. エラーがないことを確認する。

### 2-2. ATmega128へのファームウェアの書き込み
1. Z80ATmega128 BoardのAVR ISPコネクタにAVRISP mkIIを接続し、電源を入れる。
2. Microchip Studioを起動。
3. `Tools > Device Programming`を選択。
4. 以下のように値をセットし`Apply`を実行。
   | 設定       | 値         |
   |-----------|------------|
   | Tool      | AVRISPmkII |
   | Device    | ATmega128  |
   | Interface | ISP        |
5. Deivce signatureの確認  
   `Read` を実行し、値が `0x1E9702` になればOK。  
   エラーになる場合は `Interface setting` を選択し、`ISP Clock` を変更してみる。  
    ![](Fig/MS-ISPClockSetting.png)
6. FUSEの設定  
   以下のようにセットする。(設定値の詳細は[こちら](Hardware/Design.md#fuse-bits))
   ![](Fig/MS-FuseSetting.png)
7. ファームウェアの書き込み  
   `Program` を実行。 
   ![](Fig/MS-Memories.png)

### 2-3. 動作確認
1. DIP SW 1はOFFにし、CP/M起動を行ないようにする。
2. microSD Cardはまだセットしない。
3. AVR用シリアルインターフェースを端末ソフトに接続。
4. 電源ONまたはリセットボタン押下で、プロンプトが表示されることを確認。
    ```
    ATmega128 Tiny Monitor
    >
    ```
5. メモリテスト
    ```
    >test
    2000-2500
    write sum=7d80
    read  sum=7d80
    XMEM OK!
    ```
6. これでZ80ATmega128 Boardの動作確認が完了。

## 3. CP/Mの設定
Z80ATmega128 BoardでCP/M-80を動作させるための手順を説明する。  

なお、ライセンスの関係で本リポジトリにはCP/Mのソースもバイナリも置いていない。ここでは、[The Unofficial CP/M Web site](http://www.cpm.z80.de/)にある、[CP/M 2.2 BINARY](http://www.cpm.z80.de/download/cpm22-b.zip)を使用してCP/Mのディスクイメージを作成する。

### 3-1. ビルド環境の構築
BIOSのアセンブル、およびCP/Mのディスクイメージの作成はLinux環境で行う。
Windows/macOSの場合は、VS Code + Dev Containerの環境がおすすめ。

#### Linux (Debian12)の場合
1. 必要なツールのインストール
   ```
   sudo apt-get install -y wget git make unzip bzip2 g++ gcc bsdmainutils cpmtools
   ```
2. Z80のクロスアセンブラ(asxxxx)のインストール
   ```
   cd z80/toolchain
   make
   ```
3. cpmtoolsのインストール
   ```
   sudo apt-get install -y cpmtools
   ```
   `/etc/cpmtools/diskdefs`に以下の定義がない場合は追加する。([Issue#13](https://github.com/46nori/Z80Atmega128/issues/13))
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

#### VS Code + Dev Containerの場合
- [Docker Desktop](https://www.docker.com/products/docker-desktop/) と [VS Code](https://azure.microsoft.com/ja-jp/products/visual-studio-code/)をインストール
- VS Codeの`Dev Containers`プラグインをインストール
- Dev Containerでbashを起動


### 3-2. CP/MディスクイメージとmicroSD Cardの作成
1. CP/Mディスクイメージの生成 (VSCode + Dev Container環境の場合)    
  `z80/cpm22/image` で `DISK00.IMG` を生成する。これがCP/Mのディスクイメージのバイナリファイル。
    ```
    vscode@Z80ATmega128:/z80/cpm22/image$ make
    mkdir -p ./tmp
    wget -P ./tmp http://www.cpm.z80.de/download/cpm22-b.zip
    --2023-10-10 16:04:18--  http://www.cpm.z80.de/download/cpm22-b.zip
    Resolving www.cpm.z80.de (www.cpm.z80.de)... 92.205.48.95, 2a00:1169:103:9b30::
    Connecting to www.cpm.z80.de (www.cpm.z80.de)|92.205.48.95|:80... connected.
    HTTP request sent, awaiting response... 200 OK
    Length: 42510 (42K) [application/zip]
    Saving to: ‘./tmp/cpm22-b.zip’

    cpm22-b.zip                  100%[============================================>]  41.51K   124KB/s    in 0.3s    

    2023-10-10 16:04:20 (124 KB/s) - ‘./tmp/cpm22-b.zip’ saved [42510/42510]

    unzip -o -d ./tmp ./tmp/cpm22-b.zip
    Archive:  ./tmp/cpm22-b.zip
    inflating: ./tmp/CPM.SYS           
    inflating: ./tmp/DDT.COM           
    inflating: ./tmp/PIP.COM           
    inflating: ./tmp/SUBMIT.COM        
    inflating: ./tmp/XSUB.COM          
    inflating: ./tmp/ED.COM            
    inflating: ./tmp/ASM.COM           
    inflating: ./tmp/LOAD.COM          
    inflating: ./tmp/STAT.COM          
    inflating: ./tmp/DUMP.COM          
    inflating: ./tmp/DUMP.ASM          
    inflating: ./tmp/BIOS.ASM          
    inflating: ./tmp/DEBLOCK.ASM       
    inflating: ./tmp/DISKDEF.LIB       
    inflating: ./tmp/DSKMAINT.COM      
    inflating: ./tmp/READ.ME           
    rm -f ./tmp/cpm22-b.zip
    mkfs.cpm -f sdcard -b ./tmp/CPM.SYS DISK00.IMG
    dd if=/dev/zero of=DISK00.IMG bs=8192 count=1021 oflag=append conv=notrunc
    1021+0 records in
    1021+0 records out
    8364032 bytes (8.4 MB, 8.0 MiB) copied, 0.139112 s, 60.1 MB/s
    cpmcp -f sdcard DISK00.IMG ./tmp/*.* 0:
    cpmls -f sdcard DISK00.IMG
    0:
    asm.com
    bios.asm
    cpm.sys
    ddt.com
    deblock.asm
    diskdef.lib
    dskmaint.com
    dump.asm
    dump.com
    ed.com
    load.com
    pip.com
    read.me
    stat.com
    submit.com
    xsub.com
    echo `wc -c < DISK00.IMG`
    8429568
    ```
2. microSD Cardの作成
   - **FAT32**でフォーマットする。
   - ルートディレクトリに `DISK00.IMG` をコピーする。これはシステムディスクで必ず存在する必要がある。
     - `00`はドライブA:に相当する。`4`(ドライブE:)まで指定可能。
     - 例えば `DISK00.IMG` を `DISK01.IMG` としてコピーし追加すれば、B:ドライブが見えるようになる。

### 3-3. CP/M自動起動のための設定
microSD CardからCP/Mが起動できるようにするための設定を行う。

1. BIOSのビルド (VSCode + Dev Container環境の場合)  
   `z80/cpm22/bios` で `bios.ihx` を生成する。これはIntel HEX formatのファイル。
    ```
    vscode@Z80ATmega128:/z80/cpm22/bios$ make bios.ihx
    asz80 -l -o  bios.asm
    
    aslink -i bios.ihx bios.rel
    
    ASlink >> -i
    ASlink >> bios.ihx
    ASlink >> bios.rel
    ```

2. EEPROMへのBIOSの書き込み
   1. AVR用のシリアルインターフェースを端末に接続。
   2. AVR Tiny Monitorの`xload`コマンドで、Intel HEXフォーマットのBIOSをSRAM上にバイナリ展開する。AVR用端末からXMODEMで `bios.ihx` を送信する。62K CP/M用BIOSなので、バイナリは0xf200から配置される。
        ```
        >xload
        Start XMODEM within 90s...

        Received 2816 bytes.
        ```
   3. EEPROMにダウンロードしたBIOSイメージをATmega128のEEPROMの0番地に転送する。終了するまで少し時間がかかる。先頭4バイトにはBIOSの先頭アドレス(0xf200)と長さ(0x00b0)が書き込まれる。
        ```
        >esave2 0 $f200 2816
        >
        ```

### 3-4. 動作確認
  1. AVR, Z80用のシリアルインターフェースを端末に接続。
  2. CP/Mイメージファイルを書き込んだmicroSD Cardをスロットに挿入する。
  3. DIP SW 1をON(CP/M起動モード)にして、リセットボタンを押す。
  4. AVR側のシリアル端末には以下のプロンプトが表示される。  
     EEPROMに書き込まれたBIOSがSRAM上にコピーされ、BIOSがmicroSD Cardの予約トラックに書き込まれているCCP+BDOSを読み込んで、CP/Mを起動する。
     ```
     === CP/M mode ===
     BIOS: 0xf200 - 0xfcff
     
     ATmega128 Tiny Monitor
     >
     ```
     Z80側のシリアル端末に以下が表示されれば成功。
     ```
     62K CP/M-80 Ver2.2 on Z80ATmega128
     BIOS Copyright (C) 2023 by 46nori
     
     A>
     ```