## [以前の日記](Diary.md)

## 2023/7/7
- CP/Mの移植方針を決めた。
  - CP/M本体(CCP, BDOS)
    - 最初はどこかからCP/Mバイナリを持ってきて、PCであれこれ加工してメモリイメージを作成し、何らかの方法でSRAMに転送して起動させようと考えていた。と考えるだけでも面倒臭いので、他の方法を探していたが、[The Unofficial CP/M Web site](http://www.cpm.z80.de/)で[CP/M2.2のソース](http://www.cpm.z80.de/download/cpm2-asm.zip)があるのに気づいた。根性のあるおっさんがメモリイメージをディスアセンブルして、Z80のニーモニックで書き起こしたものらしい。これをそのままアセンブルできれば、前後にIPLとBIOSをリンクしてINTEL HEX出力するだけで良くなるので相当楽。実機への転送も簡易モニタでロードするだけだ。
  - BIOS
    - 最低限の初期化、Cold Boot, Warm Boot, コンソール入出力のみ作成し、起動メッセージが出るところまでを確認することにした。DISK I/Oはそのあとじっくり取り組む。
  - IPL
    - 起動時に0番地からBIOSのCold Bootを呼ぶだけ。
- まずはこの方針で行けそうか、CP/Mのソースコードのアセンブルを試金石とした。アセンブラはasz80で乗り切れそうな気がしたので、トライ&エラーでソースを修正してたら、あっさりアセンブルできてしまった。BIOSを書くためのアセンブラを最終的にどれにするか悩んでいたけど、もうこれでいいや。変更箇所は以下。
  - 16進表記の変更(例: 1AH → 0x1A)
  - 文字リテラルのシングルクオート('A') → ダブルクオート("A")に変更
  - ディレクティブの変更
    - `EQU`,`DEFB`,`DEFW` → `.equ`,`.db`,`.dw`
    - `ORG` → `.org`、さらに`.area`でセグメントの属性の指定。
    - 文字列("...")は`.str`
  - `EQU $`をその位置の示すアドレスラベルで置き換え。
    - (2023/7/12追記) `$`と同等の表現は`#.`であることがわかった。
- 修正したCP/Mのソースコードをリポジトリに取り込むのは、ライセンスのリスクがありそうなので、パッチを作成して管理することにした。ソースをダウンロードしてパッチを当て、アセンブルするMakefileを書いた。
- ちなみに62K CP/Mの設定になっている。

## 2023/7/8
- [エミュレーションデバイス仕様書](Software/EmulatedDeviceSpec.md)を更新した。
  - CONOUTの未出力データ、Console Bufferのサイズ取得機能追加。
  - 割り込み周りのレジスタ変更。
  - デバッグ用にLEDを操作できる仮想デバイスを追加。
- エミュレーションデバイス仕様に合わせて、コンソール入出力だけ対応したBIOSを記述した。しかしxloadコマンドで読み込んで実行すると暴走する？
  - BIOSの先頭アドレスである0xf200からのメモリイメージをINTEL HEXと比較してみると、正しく展開されていない。ダミーデータをbloadで読み込ませるとやはり正しく展開されていない。これが暴走の原因のようだ。
  - XMODEMの読み込みがXMEMアドレスの位置によってうまくいっていないようだ。AVR側のXMEMの書き込みルーチンを見直す必要あり。

## 2023/7/9
- INTEL HEXをメモリに展開しながらアドレスと書き込みデータをTX1に出力して確認したが問題なかった。
- しかし、メモリイメージを見直すと128byte境界でおかしなデータが紛れ込んでいる。さらにデータを書き込むと128byte先に同じデータが書き込まれことに気づいた。指定メモリブロックのR/Wチェックを行うプログラムを組み込んで測定すると上位側SRAM(0x8000-0xffff)で問題が発生する。ほぼハードの問題で確定。
- 配線を確認するとSRAMの1pinと14pinがGNDに落ちている。ポリウレタン線の被覆が剥がれてGNDラインに接触していたたのが原因。該当部分をセロテープで絶縁した。ポリウレタン線はコンパクトに配線できて便利だが、被覆の破損が目視ではわかりにくいのが辛い。もともと手配線量が多いので、デバッグ時の配線のズラしや経時変化で微妙に壊れることがある。早くプリント基板を起こしたいが、動作確認が終わらないとそれができないジレンマ。。
- ところが、これを直すとさらに悪化した。上位側SRAMへの書き込みが全くできなくなった。なぜだ？？？

## 2023/7/11
- なぜか今日はメモリ問題が発生しない。やはりどこかの接触不良なのか？
- 「マイコンピュータ No.16 CP/M活用テクノロジー(CQ出版1985年)」をじっくり読みながら、CP/M BIOSのCBOOT、WBOOT、SELDSKの実装を見直した。
  - CBOOTでブートメッセージを表示するようにした。
  - WBOOTで0x006番地にセットするBDOSのエントリポイントが6Byteずれてたバグを修正。
  - SELDSKにではHL=0で返すようにした。
- **これでやっとCP/Mが起動するようになった！**
  - ただしSELDSKでエラーを返すので、CP/Mの再起動のループになってしまう。
  - ダミーのDPHは追加したので、パラメータを見直して、SELDSKで返すようにし、DISK I/Oの繋ぎこみ実装を行えばまともに動き出すはず。

## 2023/7/12
- BIOSのDISK READ/WRITEを書き殴った。AVR側のSD CardのR/Wがまだ実装されていないので、まだデバッグはできない。
  - DISK READ
    - 512byteつまり4レコード分のリードキャッシュをサポートしている。
    - SD Cardのセクタサイズが512byteなので、セクタ境界から512byteリードする。こうしないとリードの効率が悪くなるので。BDOSから指定されるレコードは必ずしもこの境界に乗らないがリードデータには必ず含まれる。当該レコードの先頭はブロッキングファクタから計算できる。ブロッキングファクタはレコード番号を4で割った余り。
  - DISK WRITE
    - ライトキャッシュは真面目にやると大変なので、毎回書き込みを行う。
    - ライトが発生したら、リードキャッシュの状態を無効にする。

## 2023/7/13
- AVR側のSD CardのR/Wを書き殴った。INT0, INT1割り込みハンドラ内では外部SRAMアクセスができないため、R/W処理はTIMER2の周期割り込みハンドラ内で実行している。これでZ80側のBIOSとつながったはずなのだが。。。

## 2023/7/14
- BIOSのデバッグが進まなくなった。以下の現象からハードの問題っぽい。
  - 0x8000以降のメモリアクセスがうまくいっていない。簡易モニタのtestコマンドでメモリチェックを行うとチェックサムエラーが出る。
  - Z80側のプログラムが実行されていない？
  - なぜか外部SRAMアクセス時、/BUSREQに対する/BUSACKが返らない。AVR側が無限ループにハマる。
  - どうも/WAITがLowに張り付いているように見える。これが原因でZ80が割り込みサイクルから抜けられていない？BUSACKの問題もこれが原因か？
  - ロジアナで観測すると、4MHzクロックの立ち上がりとは無関係なタイミングでLowに引き戻されているように見える。
- ということで、Z80の制御線まわりの総点検を行った。
- 7/3のBUSACK問題の改修ミスを発見。/BUSACKのモニタ用LEDの接続が、74HC08の出力につながっていた。別に問題はないが、回路図上はZ80の/BUSACKにつながっているべきなので改修。
- /BUSREQをプルダウンにしていたがプルアップに変更。リセット時などAVR出力が不定になる期間にZ80を止める目的でプルダウンとしたが、リセット回路の修正でこれは不要になったので。
- WAIT生成回路の修正
  - 6/8の2nd WAIT対策で、WAIT生成回路の74HC74の出力と/IOREQのORを取った結果を/WAITとした。しかし74HCT754のCP入力には使えないことに気づいた。ここはORを取る前の74HC74の出力を使わないといけない。理由は以下。
  - 74HCT574のラッチは、Z80のIN命令(IORD)、および外部割り込みのベクタリード時に使用される。ラッチタイミングは/WAITの立ち上がりである。Z80がデータを読み込むのは/WAIT解除を認識した後のT3のタイミング。
  - 2nd WAIT対策の結果として/WAIT信号にヒゲが出るようになった。つまり/WAITの立ち上がりは2回発生する。しかし2回目のラッチは、AVRは割り込みハンドラの出口でPortAをハイインピーダンスに切り替えた後なので値は不定。また/OEもイネーブル状態なのでこの変化がそのままZ80のデータバスに現れてしまう。結果間違った値が読み込まれる可能性がある。
  - これを回避するにはヒゲのない74HC74の出力をそのまま利用する。この信号を/WAIT_0とし、回路図を修正した。

## 2023/7/18
- OUT命令のINT1ハンドラの/CLRWのパルス幅が短かったのを修正した。/CLRWがLowの期間、74HC74のQ出力はHighをキープできる。Z80が確実にWAIT解除を認識するよう、1クロック以上Highになるよう調整。時々WAITが解除されない問題に随分悩まされたが、結局原因はこれだった。
  - /CLRWAITのパルス幅 = 5CLK x 62.5ns(@AVR 16MHz) = 312.5ns > 250ns(@Z80 4MHz)

## 2023/7/19
- BIOS 起動時のエントリポイントを、WBOOTに変更。

## 2023/7/23
- BIOSのDISK I/Oまわりのデバッグ。
- CP/Mのディスクイメージをちゃんと作成してやらないと、うまく起動できなさそうなので。DPBとイメージの作成を整理することにした。

## 2023/7/30
- [cpmtools](http://www.moria.de/~michael/cpmtools/)の`mkfs.cpm`を使えば、CP/Mのイメージの作成ができるようだ。
  - GitHubの[フォーク版](https://github.com/lipro-cpm4l/cpmtools)もあるみたい。
- Debianでパッケージが用意されているので、Dev ContainerのDockerfileでデフォルトでインストールするようにした。
  ```
  apt-get install cpmtools
  ```
- 生成するイメージのDISKフォーマットは、mkfs.cpmコマンドの`-f`オプション指定する。
- DISKフォーマットは`/etc/cpmtools/diskdefs`に書かれている。別ファイルを指定することはできないっぽい。自前の設定を使うにはこのファイルに追記する必要がある。不便。
- CP/MのDPHやDPBを[ドキュメント](Software/DiskParameters.md)に整理した。
  - 参考文献
    - マイコンピュータ No.16 CP/M活用テクノロジー(CQ出版 1985年)
    - 応用CP/M 村瀬康治 (アスキー出版局 1982年)
    - [CP／Mディスクイメージ作成手順メモ](http://star.gmobb.jp/koji/cgi/wiki.cgi?page=CP%A1%BFM%A5%C7%A5%A3%A5%B9%A5%AF%A5%A4%A5%E1%A1%BC%A5%B8%BA%EE%C0%AE%BC%EA%BD%E7%A5%E1%A5%E2)
    - [AVR CPMフディスクパラメタブロックの設定変更](https://blog.goo.ne.jp/ishihara-h/e/4603e30b56557b824f237faa9f2ee29e)


## 2023/8/2
#### CP/Mディスクイメージの作成
- 8GB SD CARDの定義があったので、とりあえずこれを利用して、イメージを作成してみる。
  ```
  diskdef sdcard
    seclen 512        # 物理セクタ長
    tracks 256        # 物理トラック数
    sectrk 64         # 1トラックあたりのセクタ数 SPTとは異なるので注意
    blocksize 8192    # ブロックサイズ
    maxdir 256        # ディレクトリエントリ数(最大ファイル数)
    skew 0            # スキューファクター
    boottrk 1         # 予約トラック OFF相当
    os 2.2            # CP/M2.2
  end
  ```
- イメージファイル(DISK00.BIN)を生成する。65536バイトになった。最低限のサイズしか生成されないようだ。
  ```
  mkfs.cpm -f sdcard DISK00.IMG`
  wc -c < DISK00.IMG
  65536
  ```
- peti Fat FSは追加のライトができないので、ダミーを含む全サイズ(約8GB)をあらかじめ生成しておく必要がある。そこで`dd`を使用してダミーデータを後ろに追加する。
  ```
  dd if=/dev/zero of=DISK00.IMG oflag=append conv=notrunc bs=8192 count=1021
  ```
  - ブロックサイズ: blocksize = 8192 より、`bs=8192`
  - 総トラック数: tracks + boottrk = 256 + 1 = 257
  - SPT: seclen / 128 * sectrk = 256。
  - 論理セクタ長は128バイト(固定値)。
  - 生成されたDISK00.IMGのサイズは65536バイト。
  - 以上より追加ブロック数は、  
    `count = (257 * 256 * 128 - 65536) / 8192 + 1 = 1021`
  - `conv=notrunc`を指定しないとappendがうまく動かない。
- 次に、CP/Mの基本コマンド類をイメージに追加する。
  - [CP/M 2.2 BINARY](http://www.cpm.z80.de/download/cpm22-b.zip)をダウンロードして展開すると、CP/Mの基本ファイルのバイナリが得られる。おぉ、懐かしいファイル名だ。
    ```
    % ls cpm22-b
    ASM.COM		DEBLOCK.ASM	DUMP.COM	READ.ME
    BIOS.ASM	DISKDEF.LIB	ED.COM		STAT.COM
    CPM.SYS		DSKMAINT.COM	LOAD.COM	SUBMIT.COM
    DDT.COM		DUMP.ASM	PIP.COM		XSUB.COM
    ```
  - `cpmcp`コマンドでイメージに追加する。`0:`はユーザー番号。
    ```
    cpmcp -f sdcard DISK00.IMG cpm22-b/*.* 0:
    ```
  - 中身を確認。
    ```
    % cpmls -f sdcard dummy.img 
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
    ```
- 完成したイメージ
  - 0x0000-0x7fffまではboottrk用。いまは何も書かれていないが、最終的にはmkfs.cpm実行時に`-b`オプションでCCP+BDOSのイメージを追加しておき、BIOSのWBOOTでここからCP/Mをリロードできるようにする。
  - 0x8000からディレクトリが始まる。
  ```
  00000000  e5 e5 e5 e5 e5 e5 e5 e5  e5 e5 e5 e5 e5 e5 e5 e5  |................|
  00000010  e5 e5 e5 e5 e5 e5 e5 e5  e5 e5 e5 e5 e5 e5 e5 e5  |................|
  ...
  00007ff0  e5 e5 e5 e5 e5 e5 e5 e5  e5 e5 e5 e5 e5 e5 e5 e5  |................|
  00008000  00 41 53 4d 20 20 20 20  20 43 4f 4d 00 00 00 40  |.ASM     COM...@|
  00008010  01 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
  00008020  00 42 49 4f 53 20 20 20  20 41 53 4d 00 00 00 60  |.BIOS    ASM...`|
  00008030  02 00 03 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
  00008040  00 43 50 4d 20 20 20 20  20 53 59 53 00 00 00 3c  |.CPM     SYS...<|
  00008050  04 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
  00008060  00 44 44 54 20 20 20 20  20 43 4f 4d 00 00 00 26  |.DDT     COM...&|
  00008070  05 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
  00008080  00 44 45 42 4c 4f 43 4b  20 41 53 4d 00 00 00 50  |.DEBLOCK ASM...P|
  00008090  06 00 07 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
  000080a0  00 44 49 53 4b 44 45 46  20 4c 49 42 00 00 00 31  |.DISKDEF LIB...1|
  000080b0  08 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
  000080c0  00 44 53 4b 4d 41 49 4e  54 43 4f 4d 00 00 00 12  |.DSKMAINTCOM....|
  000080d0  09 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
  000080e0  00 44 55 4d 50 20 20 20  20 41 53 4d 00 00 00 21  |.DUMP    ASM...!|
  000080f0  0a 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
  00008100  00 44 55 4d 50 20 20 20  20 43 4f 4d 00 00 00 04  |.DUMP    COM....|
  00008110  0b 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
  00008120  00 45 44 20 20 20 20 20  20 43 4f 4d 00 00 00 34  |.ED      COM...4|
  00008130  0c 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
  00008140  00 4c 4f 41 44 20 20 20  20 43 4f 4d 00 00 00 0e  |.LOAD    COM....|
  00008150  0d 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
  00008160  00 50 49 50 20 20 20 20  20 43 4f 4d 00 00 00 3a  |.PIP     COM...:|
  00008170  0e 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
  00008180  00 52 45 41 44 20 20 20  20 4d 45 20 00 6b 00 01  |.READ    ME .k..|
  00008190  0f 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
  000081a0  00 53 54 41 54 20 20 20  20 43 4f 4d 00 00 00 29  |.STAT    COM...)|
  000081b0  10 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
  000081c0  00 53 55 42 4d 49 54 20  20 43 4f 4d 00 00 00 0a  |.SUBMIT  COM....|
  000081d0  11 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
  000081e0  00 58 53 55 42 20 20 20  20 43 4f 4d 00 00 00 06  |.XSUB    COM....|
  000081f0  12 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
  00008200  e5 e5 e5 e5 e5 e5 e5 e5  e5 e5 e5 e5 e5 e5 e5 e5  |................|
  00008210  e5 e5 e5 e5 e5 e5 e5 e5  e5 e5 e5 e5 e5 e5 e5 e5  |................|
  ...
  00809fe0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
  00809ff0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
  ```

## 2023/8/6
- SD CardをFAT32でフォーマットし、イメージファイルを`DISK00.IMG`としてコピーしておく。
- WBOOTで、トラック1からCCP+BDOSをリロードできるようにした。
- DPBを定義しDISK READに関しては一通りの機能を実装したつもりなのだが、、、
- BIOSのBOOTからCCPを起動(CCPのエントリ+3にジャンプ)するが、ディレクトリの読み込み後に、コンソール出力に`^@`(制御コード)が延々と表示される。この原因がさっぱりわからない。Z80側のデバッグはOUT命令くらいしか使えないので、AVR側からZ80にブレークポイントを張れるようにした。

#### ブレークポイント機能の追加
- 簡易モニタに以下のコマンドを追加した。
  - `brk <adr>` : adrに0xff(RST 7)を書き込む。Z80がそのadrを実行すると0x38にトラップし、HALT状態になる。BIOS起動時に38番地にはBIOS内のハンドラへのJP命令をセットしてある。ハンドラ内ではスタック上の戻り先を1命令前に戻し、ブレークポイント達したことをOUT命令でAVRに通知し、HALT状態で待機する。
  - `del <adr>` : adrに書き込んだ命令を元に戻し、ブレークポイントを削除する。
  - `cont` : AVRからZ80に割り込みをかけることで、HALTを解除し、ブレークポイント位置に復帰する。~~復帰先の命令は0xffのままなので、実行前に`del`で命令の復帰とブレークポイント削除を行っておかないと、再度ブレークがかかってしまうので注意。~~ (2023/8/8 ブレークポイントを消すことなくcontできるようにした。)
  - `ib` : ブレークポイントの一覧(アドレスと0xffセット前の命令)を表示する。
- ブレークするとAVRのコンソールに以下が表示される。(2023/8/8 ブレークアドレスは、下位だけでなく上位アドレスも表示するようにした。)
  - `>>>Break! at ブレークアドレス` → 登録したブレークポイントにヒットした
  - `>>>Break! at ブレークアドレス (unexpected)` → 暴走などでRST 7(0xff)を実行

## 2023/8/8
- [エミュレーションデバイス仕様書](Software/EmulatedDeviceSpec.md)を更新。ブレークポイント機能のためのポート0x1d, 0x1eを追加。これで余剰ポートがなくなってしまった。まぁ、足りなくなったらもう1ビット増やせばいいのだけど。
- ブレークポイントの振る舞いを少し賢くしたので、本格的にデバッグすることにしよう。

## 2023/8/9
- ブレークポイント機能を使って、CCP/BDOSの動きを追っていった。
- まず、DDA1の`CALL ENTRY`の前後にブレークを張って観察する。この呼び出しで`^@`が大量表示された。ここはBDOS 10番コール(RDBUFF)で、コンソールからの1行入力。
  ```
  DC06 7F                   53 INBUFF:  .db 127   ;length of input buffer.
  DC07 00                   54          .db 0     ;current length of contents.
  DC08 43 6F 70 79 72 69    55 	.str	"Copyright"
  ```
  ```
                            292 ;
                            293 ;   Get here for normal keyboard input. Delete the submit file
                            294 ; incase there was one.
                            295 ;
   DD96 CD DD DD      [17]  296 GETINP1:CALL	DELBATCH	;delete file ($$$.sub).
   DD99 CD 1A DD      [17]  297 	CALL	SETCDRV		;reset active disk.
   DD9C 0E 0A         [ 7]  298 	LD	C,10		;get line from console device.
   DD9E 11 06 DC      [10]  299 	LD	DE,INBUFF
   DDA1 CD 05 00      [17]  300 	CALL	ENTRY
   DDA4 CD 29 DD      [17]  301 	CALL	MOVECD		;reset current drive (again).
  ```
  - コンソール入力がないのに、なぜかリターンしてきてDDA4でブレークする。入力文字数がセットされるDC07の値は127になっている。そして確かに127個の`^@`が表示されている。(ちなみにこの値はバッファサイズに連動している。INBUFFの127を3に変えると、DC07の値も3になった。)  
  - DC08からの文字列バッファは0で埋められている。初期値の`Copyright`が消えているので、確かに上書きされている。
- 以上より、コンソール入力周りが怪しい。無入力にも関わらず、入力として受け取ったダミー値をバッファに目一杯書き込んでいるように見える。この条件ではAVRはIN命令で0を返すので辻褄があう。BDOSのバグとは考えにくいので、BIOSの入力チェック(CONSTS)がおかしいのかも？
- DDA1のCALL ENTRYはBDOSのRDBUFFの呼び出しなので、ここを読み進める。
  ```
                          1512 ;
                          1513 ;   Function to execute a buffered read.
                          1514 ;
  E5E1 3A 0C E7      [13] 1515 RDBUFF:	LD	A,(CURPOS)	;use present location as starting one.
  E5E4 32 0B E7      [13] 1516 	LD	(STARTING),A
  E5E7 2A 43 E7      [16] 1517 	LD	HL,(PARAMS)	;get the maximum buffer space.
  E5EA 4E            [ 7] 1518 	LD	C,(HL)
  E5EB 23            [ 6] 1519 	INC	HL		;point to first available space.
  E5EC E5            [11] 1520 	PUSH	HL		;and save.
  E5ED 06 00         [ 7] 1521 	LD	B,0		;keep a character count.
  E5EF C5            [11] 1522 RDBUF1:	PUSH	BC
  E5F0 E5            [11] 1523 	PUSH	HL
  E5F1 CD FB E4      [17] 1524 RDBUF2:	CALL	GETCHAR		;get the next input character.
  ```
  - E5F1の`CALL GETCHAR`(1文字入力)で以下につながる。バッファはまだ空なので`JP CONIN`が呼ばれる。これはBIOSのCONIN(1文字入力)呼び出しのことだ。
  ```
                          1345 ;
                          1346 ;   Get an input character. We will check our 1 character
                          1347 ; buffer first. This may be set by the console status routine.
                          1348 ;
  E4FB 21 0E E7      [10] 1349 GETCHAR:LD	HL,CHARBUF	;check character buffer.
  E4FE 7E            [ 7] 1350 	LD	A,(HL)		;anything present already?
  E4FF 36 00         [10] 1351 	LD	(HL),0		;...either case clear it.
  E501 B7            [ 4] 1352 	OR	A
  E502 C0            [11] 1353 	RET	NZ		;yes, use it.
  E503 C3 09 F2      [10] 1354 	JP	CONIN		;nope, go get a character responce.
  ```
  - おい？ちょっと待て。BIOSのキー入力チェック(CONSTS)で入力チェックしてから呼ばれるんじゃないのか？まさかCONINてブロッキングAPIだったの？
    - 答え：　応用CP/M P.11のCONINの説明:「このルーチンは、入力がない場合は内部で入力があるまでループさせること。」
- **原因判明。CONINはブロッキングAPIだった。**
- ということでBIOSのCONINを入力があるまで待つように書き換えたところ、、、**動いた！**
  ```
  CP/M-80 Ver2.2 on Z80ATmega128

  a>dir
  A: ASM      COM : BIOS     ASM : CPM      SYS : DDT      COM
  A: DEBLOCK  ASM : DISKDEF  LIB : DSKMAINT COM : DUMP     ASM
  A: DUMP     COM : ED       COM : LOAD     COM : PIP      COM
  A: READ     ME  : STAT     COM : SUBMIT   COM : XSUB     COM
  A: ZORK1    COM : ZORK1    DAT
  a>stat
  A: R/W, Space: 0k
  ```
- だが、まだ動作が不安定。`dir`表示が糞づまったりするし、トランジェントコマンドが起動できたりできなかったり。`stat`の結果も怪しい。SD Card上のDISKイメージや、DISK IOにまだバグがありそう。いずれにしても、ブレークポイントをサポートしたことで、デバッグが相当楽になった。

## 2023/8/12
- ブレーク時にZ80レジスタも見れるようにした。やはりスタックとレジスタが見られないのは辛いので。復帰動作に影響が出ないよう、レジスタやスタックを壊さず保存するのは神経を使う。

## 2023/8/13
- DISK I/OのエミュレーションデバイスのI/Oポート数が多いので見直すことにした。DISK位置に3ポート、バッファ、長さの設定に2ポート分確保していたが、それぞれ1ポートにする代わりにシーケンサーを用意し、Highバイトから複数回OUTして設定するようにした。同一ポートをINするとシーケンサーがリセットされる。[エミュレーションデバイス仕様書](Software/EmulatedDeviceSpec.md)を更新した。
- 変更量が多いので [feature/refactoring-portio](https://github.com/46nori/Z80Atmega128/tree/feature/refactoring_portio) ブランチで作業する。

## 2023/8/14
- 動作が不安定な原因が割り込みまわりにあるところまでは見えてきたが、まだよくわからない。
  - DISK READの割り込みがZ80かからない時がある。
  - DISK READ終了を監視するループでの暴走(実際にはRST7のハンドラが呼ばれてブレイクする)。このループはDISK READ完了割り込みのハンドラ内でメモリにセットするフラグをLD命令で監視するだけのコード。ブレイクアドレスをメモリダンプしても0xffになっていない。

## 2023/8/15
- Z80側の割り込みをHALTで受けるように変更したら動作が安定した。だがHALTでの待ち合わせはMode2割り込みによる個別処理を否定する実装なので、この変更は不本意。あとで原因を追求したい。
  - 変更前
    - [Z80] ステータスレジスタを確認。機能実行中であれば割り込みフラグがセットされるのをループで監視。
    - [AVR] 機能の実行が完了したら、あらかじめ指定された割り込みレベルでZ80に割り込みをかける。
    - [Z80] 割り込みがかかったらフラグをセットする。監視ループでフラグを認識したら、再度ステータスレジスタを確認してエラーチェックしリターンする。
  - 変更後
    - [Z80] ステータスレジスタを確認。機能実行中であれば**HALTする。**
    - [AVR] 機能の実行が完了したら、あらかじめ指定された割り込みレベルでZ80に割り込みをかける。
    - [Z80] **割り込みがかかるとHALTが解除される**。再度ステータスレジスタを確認してエラーチェックしリターンする。
  - ちなみにステータスレジスタをIN命令でポーリングすることで、機能の実行完了を確認することもできるが、多数の割り込みがかりAVRに異常な負荷がかかる。このため割り込みを待ってからステータスを確認するようにしている。
- この変更によりREADの途中で暴走する頻度は極端に減った。しかし画面出力の糞詰まりはまだ時々発生する。
- `^C`によるWBOOTの動きが謎。以下のCCP+BDOSのリロードを何度も繰り返して戻ってこない。SELSDKとREADの両方が呼ばれているのが意味不明。
  ```
  ###SELSDK: DISK00.IMG
  DSK_ReadPos=00000000
  DSK_ReadBuf=dc00
  DSK_ReadLen=1600
  READ:0->1
  >>>READ:000000 : 00

  ###SELSDK: DISK00.IMG
  DSK_ReadPos=00000000
  DSK_ReadBuf=dc00
  DSK_ReadLen=1600
  READ:0->1
  >>>READ:000000 : 00

  ...
  ```
  - 他のBIOS実装を眺めてみたら、SETDMAでDMAアドレスの初期値を0x80にセットしているので真似してみた。一瞬良くなったようにも見えたがまだダメみたいだ。
- 気分転換に[Commercial CP/M Software](http://www.retroarchive.org/cpm/)にあるソフトを組み込んだイメージを作成した。ZORK I,II,IIIも入れてみた。
  ```
  62K CP/M-80 Ver2.2 on Z80ATmega128
  BIOS Copyright (C) 2023 by 46nori

  a>dir
  A: -CCSCPM  251 : ASM      COM : BIOS     ASM : CBIOS    ASM
  A: CCBIOS   ASM : CCBOOT   ASM : CCSINIT  COM : CCSYSGEN COM
  A: CPM24CCS COM : DDT      COM : DEBLOCK  ASM : DISKDEF  LIB
  A: DUMP     ASM : DUMP     COM : ED       COM : FILE_ID  DIZ
  A: GENMOD   COM : LOAD     COM : MOVCPM   COM : PIP      COM
  A: RLOCBIOS COM : STAT     COM : STDBIOS  ASM : SUBMIT   COM
  A: SYSGEN   COM : XSUB     COM : ZORK1    COM : ZORK1    DAT
  A: ZORK2    COM : ZORK2    DAT : ZORK3    COM : ZORK3    DAT
  a>
  ```
  ためしにZork Iを起動してみる。
  ```
  a>zork1
  ZORK I: The Great Underground Empire
  Copyright (c) 1981, 1982, 1983 Infocom, Inc. All rights
  reserved.
  ZORK is a registered trademark of Infocom, Inc.
  Revision 88 / Serial number 840726

  West of House
  You are standing in an open field west of a white house, with
  a boarded front door.
  There is a small mailbox here.

  >
  ```
  うぉおおおおーーー。動いた!!

  ```
  a>zork1
  ZORK I: The Great Underground Empire
  Copyright (c) 1981, 1982, 1983 Infocom, Inc. All rights
  reserved.
  ZORK is a registered trademark of Infocom, Inc.
  Revision 88 / Serial number 840726

  West of House
  You are standing in an open field west of a white house, with
  a boarded front door.
  There is a small mailbox here.

  >take mailbox
  It is securely anchored.

  >look around
  West of House
  You are standing in an open field west of a white house, with
  a boarded front door.
  There is a small mailbox here.

  >open door
  Opening the small mailbox reveals a leaflet.

  >read leaflet
  How does one read a door?

  >quit
  Your score is 0 (total of 350 points), in 5 moves.
  This gives you the rank of Beginner.
  Do you wish to leave the game? (Y is affirmative): >Y
  ```
  CP/Mのプログラムなので 終了時にWBOOTが呼び出されるが、例のCCP+BDOS reloadの無限ループから戻ってこない。(謎に戻ってくるときもあるのだが...)  
  でもモチベーションが爆上がりした。
- しばらくZork Iで遊んでみたが、途中で暴走した(泣)。やはりまだ不安定。
  ```
  >>>>Break! at $2acc (unexpected)
  AF=$c384 [Z:0 C:0] I=$f3
  BC=$00c1 DE=$0000 HL=$0034
  IX=$efff IY=$fd77 SP=$e734
  (SP+0)=$2acc
  (SP+2)=$2200
  (SP+4)=$2155
  ```

## 2023/8/16
- CONOUTの糞詰まり問題を片づけることにした。
  - AVR側のTX1への出力は9600bpsなので1文字出力には時間は0.83msかかる。そこで1msの周期割り込みハンドラでリングバッファからデキューして出力することとした。つまり1ms毎に出力される。
  - Z80側からのオーバーランを防ぐため、出力リングバッファがfullになったときにZ80に割り込みをかける。Z80は割り込みがかかったらフラグをセットする。1文字出力ルーチンでは、初めにフラグをチェックし、セットされていればバッファに空きが出るまでループ後、1文字出力を行う。
- **突然AVR側が以下のエラーでリプログラムができなくなった。**  
  Error
  ```
  Failed to enter programming mode. ispEnterProgMode: Error status received: Got 0xc0, expected 0x00 (Command has failed to execute on the tool)

  Unable to enter programming mode. Verify device selection, interface settings, target power, security bit, and connections to the target device.
  ```
  Details
  ```
  Timestamp:	2023-08-16 17:09:06.826
  Severity:		ERROR
  ComponentId:	20100
  StatusCode:	1
  ModuleName:	TCF (TCF command: Device:startSession failed.)

  Failed to enter programming mode. ispEnterProgMode: Error status received: Got 0xc0, expected 0x00 (Command has failed to execute on the tool)
  ```
## 2023/8/18
- エラーの原因がさっぱりわからん。ライターの故障を疑って別のライターで試してみたが、基本、状況は変わらなかった。以下のエラーになることもあった。
  ```
  Unable to enter programming mode. The read device ID does not match the selected device or any other supported devices.

  Please verify device selection, interface settings, target power and connections to the target device.
  ```
  Details
  ```
  Timestamp:	2023-08-18 21:25:25.701
  Severity:		ERROR
  ComponentId:	20100
  StatusCode:	131101
  ModuleName:	TCF (TCF command: Device:startSession failed.)

  Unexpected signature 0x00000102 (expected 0x001e9702).
  ```
  - ちなみにAVRISPmkIIの純正品は2016年にディスコンになっていた。純正の代替品は[ATMEL-ICE](https://www.microchip.com/en-us/development-tool/atatmel-ice)しかないようだ。しかし機能も価格もオーバースペックなので、ちょっと怪しいが[互換品(2699円)](https://www.amazon.co.jp/gp/product/B07L3DT86R/ref=ppx_yo_dt_b_asin_title_o00_s00?ie=UTF8&psc=1)をAmazonで購入した。

- AVRのGPIOの空きに、以下を割り当てた。
  - PE5,6,7を出力に設定し、74HC32のORをバッファにしてLED1,2,3を接続。
  - PG3,4/PE2,3を入力に設定し、DIP SW1,2,3,4を接続。(プロトタイプは基板に4連DIP SWを乗せるスペースが取れなかったので、スライドSWを4つ接続した。)
- Z80のHALTを74HC125のバッファを介してLEDを接続。

## 2023/8/19
- ISP周りに接続されているICの故障も疑い、関係する74HC125, 74HC04を交換したが状況は変わらず。
- FUSE bitの変更で治ったという[StackOverflowの記事](https://stackoverflow.com/questions/54942770/avr-studio-error-got-0xc0-expected-0x00)を見つけた。
   - セラロックの配線を外し、ブレッドボードで4MHzクロックから1MHzを生成してXTAL1に供給してみたが、ISP接続は改善されなかった。
   - たまたま認識したタイミングを使って以下のFUSE設定(0xf7)に変更したが、かえって状況が悪化してしまった。
  
    |          | bit |現在値(0xcf)|推奨値(0x7f)|
    |----------|-----|------|-----|
    | BODLEVEL |  7  | 1    | 1   |
    | BODEN    |  6  | 1    | 1   |
    | SUT1-0   | 5-4 | 00   | 11  |
    | CKSEL3-1 | 3-1 | 111  | 011 |
    | CKSEL0   |  0  | 1    | 1   |

## 2023/8/20
- 最後の手段でATmega128の交換を実施。  
  が、認識されない。Microchip Studioの再インストールも関係なし。MOSIの接続も確認済み。  
  詰んだなぁ... ハードもAVRのFuse設定も変えていなかったのに、なぜ突然こんな状況になってしまったのだろう。8/16までは非常に安定して認識やリプログラムができていたのに。
  ```
  Failed to enter programming mode. Error received from tool: 
  Connection failed on the data line (MOSI) 

  Unable to enter programming mode. Verify device selection, interface settings, target power, security bit, and connections to the target device.
  ```
  Details:
  ```
  Timestamp:	2023-08-20 22:14:56.550
  Severity:		ERROR
  ComponentId:	20100
  StatusCode:	1
  ModuleName:	TCF (TCF command: Device:startSession failed.)

  Failed to enter programming mode. Error received from tool: 
  Connection failed on the data line (MOSI) 
  ```
  Microchip StudioのOutput:
  ```
  22:14:56: [ERROR] Failed to enter programming mode. Error received from tool: 
  Connection failed on the data line (MOSI) 
  , ModuleName: TCF (TCF command: Device:startSession failed.)
  22:14:56: [ERROR] Failed to enter programming mode. Error received from tool: 
  Connection failed on the data line (MOSI) 
  , ModuleName: TCF (TCF command: Device:startSession failed.)
  22:14:56: [ERROR] Failed to enter programming mode. Error received from tool: 
  Connection failed on the data line (MOSI) 
  , ModuleName: TCF (TCF command: Device:startSession failed.)
  ```
- 上記のエラーを検索したら、Microchipのknowledge baseで ["Unable to enter programming mode" while using ISP interface](https://microchip.my.site.com/s/article/Unable-to-enter-programming-mode-while-using-ISP-interface) という記事を見つけた。クロックに言及されている。[Using crystals and ceramic resonators](https://microchip.my.site.com/s/article/Using-crystals-and-ceramic-resonators) あたりの記事も確認してヒントを探してみるか。ちなみにセラロックの出力をオシロで確認したが、きれいな16MHzの正弦波が出力されていたので信号自体は問題ないと思われる。
- あるいは[AVR042: AVR Hardware Design Considerations](https://ww1.microchip.com/downloads/en/Appnotes/atmel-2521-avr-hardware-design-considerations_applicationnote_avr042.pdf)も読んでISPの共存方法を見直してみるか。
- 高電圧プログラミング([How to perform High Voltage Programming on AVR devices](https://microchip.my.site.com/s/article/How-to-perform-High-Voltage-Programming-on-AVR-devices))というのもあるみたいだが、パラレル書き込みのようなので他の信号との共存が難しそう。そもそもATmega128では使えるのか?

## 2023/8/26
- 以下の実験を行ったが、どれも効果なし。全く改善しない。
  - AVRISP mkIIを互換品に交換。
  - Microchip Studioで、Interface setting>ISP ClockでAVRISP mkIIのクロックを下げた。
  - MISO/MOSI/CLKを3ステートゲートで切り替えているが、[AVR042: AVR Hardware Design Considerations](https://ww1.microchip.com/downloads/en/Appnotes/atmel-2521-avr-hardware-design-considerations_applicationnote_avr042.pdf)の4.1.1に抵抗でMISO/MOSIを共存させる方法が載っていた。5V電源なので510オームで実験した。
  - Windows11をクリーンインストールし、Microchip Studioを入れ直した。

## 2023/8/27
- ライターの問題か切り分けるため以下を実施。
  - 新品のATmega128をTQFP変換基板にマウント。
  - 電源とISPだけ接続して認識できるか確認する。
  - 結果、ATMEL純正AVRISP mkIIはNG。**互換機では認識できた。**
- ISPのCLK出力が74HC125に入力されているが、もしかしてCMOSトレラントでないかもしれないので、10Kの抵抗でプルアップしてみたところ、微妙に間違ったDevice signatureが取れるようになった？
- そこでISP Clockを64KHz以下に落としてみたところ、何とか認識するようになった。デフォルトの128KHzに戻すとエラーになる。
- すかさずFUSEを以下に設定。
  - EXTENDED: 0xFF
  - HIGH: 0xC1
  - LOW: 0x7F
- 約二週間ぶりにリプログラムも成功した。
- **AVRISP mkII互換機かつISP Clockを64KHz**であれば動くようなので、これでしばらく様子を見ることにしよう。

## 2023/9/10
- 久しぶりにデバッグに復帰。CONOUTの見直しを行った。
- TX1出力を1msのタイマ割り込みで実行するのは間に合わない場合がありそう。2ms周期に変更した。
- DISKIOを実行するTIMER2の割り込みハンドラは時間がかかる。デフォルトのISRの設定だと割り込み禁止時間が長くなるため、ISR_NOBLOCKオプションで多重割り込みを許すようにした。
  ```
  ISR(TIMER2_COMP_vect, ISR_NOBLOCK) {
  ```
- CONOUT出力は改善されたが、まだ糞詰まる場合がある。

## 2023/9/12
- なんか煮詰まってきたので、気分転換にPCBに手をつけ始めた。回路の致命的なバグがあるとものすごい手戻りになりそうな気がするが大丈だろう。気分転換だし。
- さっそく配線のミスを見つけた。動作に支障はないがZ80の外部ピンの対応付けが一部間違っていた。
- さらにAVRの電源未接続も発見。KiCadのATmega128のシンボルライブラリが、なぜかPin52,63Pが定義されているにもかかわらず非表示にされていた。64pinの割に電源とGNDが一対しかないのが気になっていたが、KiCadのシンボルを信じてデーターシートをちゃんと確認していなかった。リプログラムが不安定になるのは、もしかしてこれが原因だったのかも。(2023/9/30: あまり関係なさそう。)

## 2023/9/18
- 9/16からの連休を使って基板を完成させた。
- 基板製造の発注
  - PCB製造メーカーを比較した。国産は5万円弱かかるのに対し中国勢は$100程度。
  - [PCBway](https://www.pcbway.jp/)にした。初回クーポン($5)適用で、送料込み$111.81。中国だがオンラインサイトはしっかりしてとてもわかりやすい。ガーバーデータをアップロードするとオンラインレビューが実行され、承認されると購入手続きになる。非常に簡単。

## 2023/9/22
- PCB製作作業を[こちら](KiCad-PCB.md)にまとめた。
  - 出荷までは3日だった。
    ```
    2023/09/18 21:25:26 発注
    2023/09/19 15:59:37 製造指示
    2023/09/21 18:05:44 出荷
    ```
  - 中国本土からDHLによる輸送、通関で手元に着荷するのは9/26の予定。

## 2023/9/23
- なんと午前中に基板が到着。発注から5日で手元に届いた。
- さっそくパーツを実装し動作確認を行った。
- 実装手順は以下。背の低いパーツから実装する。
  1. ATmega128
  2. microSD Adopter
  3. カーボン抵抗
  4. 積層セラミックコンデンサ
  5. テスト端子
  6. ICソケット
  7. LED
  8. DIP SW
  9. タクトスイッチ(RESETボタン)
  10. 水晶発振器、セラミック発振子
  11. DCジャック、電源ターミナル
  12. ショートピン
  13. 電解コンデンサ
  14. USBシリアル変換モジュール

- 動作確認手順
  1. 電源のショートをテスターで確認
  2. 全ICのVCC/GNDの接続をテスターで確認
  3. DC5Vを接続し、POWER LEDが点灯すること、テストピンで+5Vが生成されていることを確認。
  4. 以下を装着し、AVRISP mkIIを使用してATmega128のデバイスが認識できるか確認。
    - U1(74HC125), U4(74HC04), U11(74HC08)
    - Microchip StudioからFUSEの設定を行う。
  5. AVRのファームウェアを焼き込み。
  6. 簡易モニタ起動の確認。
  7. `test`コマンドでメモリチェック。
  8. `xload`でCP/Mの読み込み。
  9. `reset`でCP/M起動確認。
  - 6.まで確認できた。外部メモリ、microSDへのアクセスがうまくいっていない。

- 発見したPCBのバグと対策
  - 電源用の2回路２接点トグルスイッチの端子のピッチが異なる -> 小型スライドスイッチを2つ並べて実装
  - R1とAVR ISP用ボックスヘッダのクリアランスがない -> R1は裏面に実装することで回避
  - C23とAVR ISP用ボックスヘッダのクリアランス不足 -> 実装可能だがもう少し離したい
  - C23に1uのシルクがない
  - AE-CH340E-TYPEC(USBシリアル変換モジュール)のフットプリントの上下配置が逆 -> モジュールを裏返しに実装する
  - AE-CH340E-TYPECを実装するとUART CH0/1のシルクが隠れてしまう。モジュールの外形線の外にも記載する

## 2023/9/24
- 外部メモリアクセスとmicroSDのアクセス問題は、ATmega128のハンダ不良が原因だった。
- Z80の動きがおかしい
  - 0番値にHALT命令が書き込まれているにもかかわらず、HALTのLEDが点灯しない。
  - 結線をテスターで確認したが、問題なさそう。
  - オシロでZ80の制御信号をあたってみると、/MREQや/M1は出力されているが、/RDがHのまま変わらない。/HALTもHのまま。つまりメモリからHALT命令がフェッチできていない。なぜこんなことが起こる？
  - 暴走しているのに、/WRがHのまま変化しないのも怪しい。
- PCBのパターンを調べてみたところ、**AVRのPortGの/RD,/WRと、Z80の/RD,/WRが接続されている！**
  - なぜこんなことが起こったか？
  - おそらく回路図エディタのローカルラベルとグローバルラベルの振る舞いについて誤解していたのが原因。
    - AVR側のSRAMアクセス用のXMEM信号PG0, PG1にそれぞれ`/WR`, `/RD`の**ローカルラベル**をつけていた。GPIOと共用ピンなので回路図で見やすいようにするため。
    - 一方、Z80のメモリ信号にも`/WR`, `/RD`の**グローバルラベル**をつけていた。
    - ローカル、グローバルラベルは個別に扱われるものと思い込んでいたが、ネット上は同一信号とみなされてしまうようだ。
    - 回路図上は、ATmega128のシンボル設定でPG0,1のピン属性は双方向になっている。このためERCでは出力同士の衝突検出はされない。電気的に問題なければDRCも当然パスしてしまう。
    - 手配線していれば気づいたのだろうけど、配線量が多く自動配線したこと。それを信じ込んで目視で配線チェックサボったのが流出原因。もちろん自分が悪いのだが、ツールの使い方の誤解が原因なので、気づくのは難しかったなぁ。配線面は表と裏だけなので、パターンカットと手配線で修正できないこともないが、ちょっとこれはショックだ。

## 2023/9/27
- 気を取り直してプリント基板の版下とにらめっこ。8箇所のパターンカットと、6本のパッチで回避可能なことがわかったので、寝た。
- [表面の改修方法](Hardware/PCB/PCB1.0-FP-Errata.pdf)
- [裏面の改修方法](Hardware/PCB/PCB1.0-BP-Errata.pdf)

## 2023/9/28
- プリント基板を改修し、無事、動作することを確認した。
- `/RD`,`/WR`以外の結線エラーはなかったようで一安心。
- 動作が全体的に安定している気がする。なぜかコンソール出力の糞詰まり現象が発生しなくなっている？
- 当初は純正AVRISP mkIIが使えるようになった。が、使っているうちにまたダメになった。互換品はクロックを下げれば使えるようになる。これは試作基板の時と同じ。ISPまわりの挙動は相変わらず謎だ。

![](Fig/PCB2.jpeg)
 
## 2023/09/30
- プリント基板でソフトのデバッグ再開。
- CP/Mのシステムリロード(`^C`入力)でCCP+BDOSのロードが無限ループしてしまう問題があった。`Reloaded CPP+BDOS.`のメッセージ出力後にDISK READするとこの問題が回避ができた。だが理由がわからない。(2023/10/02 問題の回避にはなっていなかった。たまたま動いていただけ。根本原因は後述。)
- DISK WRITEで問題が発生しているようだ。B:ドライブのファイル削除やファイルコピーでエラーが発生し、ディスクイメージが壊れる。WRITE系のデバッグはほとんどやってないので、まぁ仕方がない。

## 2023/10/01
- DISK WRITEのデバッグを行った。
  - 書き込みが成功する場合と失敗する場合があるので、Peti Fat FSのマニュアルを読み直すと、こんな制限があった。つまり両者共に512バイトのセクタ境界に合うように書き込まないといけない。
   - [Write operation can start/stop on the sector boundary only.](http://elm-chan.org/fsw/ff/pf/write.html)
  - 書き込み位置直前のセクタ境界までlseekし、先頭と終端のセクタはリード・モディファイ・ライトするようにAVR側のファームを書き換えた。リードの後にポジションを巻き戻すことを忘れないこと。これで、ファイル削除、`SAVE`コマンドなどが動くようになった。
  - `era *.*`で全ファイル削除もできたが、再起動後に`dir`するとファイルの残骸が表示されるので、まだ詰めが甘いかも。
  - 全体的にアクセスが遅い。BIOS側で遅延書き込みをサポートするか、セクタサイズを512bytesに変更した方がいいかもしれない。
- BIOSの`SELDSK`の修正
  - エラー発生時にはにはHL=0でリターンするのが仕様だが、0004Hに書き込まれているカレントディスクも変更しておく必要がありそう。例えば未サポートディスクへのアクセスでエラーになると、カレントドライブが同じままなので、そのディスクをアクセスし続けて正常復帰できなくなる。
  - これはundocumentedと思われる仕様。

## 2023/10/02
- CP/Mのシステムリロード(`^C`入力)で暴走することがあった。(例えば`ZORK1.COM`終了時には`JP 0`しているはずで`WBOOT`が呼ばれる。この時に暴走していた。)
  - 原因はスタックポインタを安全な値にセットしていなかったから。ユーザブログラムはCCP+BDOSを書き潰して良いので、この領域をSPが指している状態でCCP+BDOSをロードすると確実に暴走する。対策は、SPをCCPの先頭アドレスにセットすること。`WBOOT`の前頭でSPをセットするように変更。
- AVRのDISK I/Oの高速化
  - Peti Fat FSは、`file_system.fptr`に現在位置を保持している。R/Wの際に、アクセス位置がfile_system.fptrと異なる時だけ`pf_lseek()`するようにした。これで連続アクセス時のオーバーヘッドが軽減されるはず。
  - SelectDisk()で行っていた`pf_lseek(0)`をやめた。実際にR/Wする時に必要に応じて`pf_lseek()`するから。CP/M BIOSの`SELDSK`のオーバーヘッドを減らしたかったのだが、あまり効果はなかった。
- DISK WRITEはだいぶ安定したが、以下の問題がある。
  ```
  a>b:
  b>era *.*
  ALL (y/n)?y
  b>dir
  No file
  b>^C
  Reload CCP+BDOS.

  b>dir
  ゴミが表示される
  ``` 
  もう一度`era *.*`した後に`^C`すると、以後はNo fileになる。
- ファイルコピーにすごく時間がかかる。たぶん、ドライブを跨ぐとSELDSKが発生し、その度にPeti Fat FSで`pf_open()`を行うことになる。ここがボトルネックになっていると思われる。
  ```
  a>pip b:=zork?.*

  COPYING -
  ZORK1.COM
  ZORK1.DAT
  ```

## 2023/10/03
- セクタ境界で`pf_write(0,0,&bw)`で書き込み終了を行なっていなかったので追加。

## 2023/10/04
- ライトデータの残量計算(len)が間違っていたので修正。
- 2023/10/02のゴミ問題は解決。PIPのファイルコピーも安定して動くようになった。
- ライト操作はまだ遅い。CP/M側からは128byte単位でライト要求が来るのが原因。リードと同様BIOS側で512バイトのブロッキング・デブロッキングをサポートすべきか。

## 2023/10/05
- DISK WRITEの確認継続。ZORK1で、SAVE/RESTOREコマンド動作した。生成されるZORK1.SAVも問題なし。ゴミファイルなどの生成も見られない。かなり安定したと思われる。
- CP/MはBIOSのSELDSKを頻繁に発行する。AVR側は`pf_open()`を実行するが、これに時間がかかる。無駄なSELDSKも見られるので、同じDISK番号を指定されたら`pf_open()`を実行しないようにした。A:での`^C`は早くなった。

## 2023/10/06
- 46nori以外は、Pull Requestなしでmainブランチにpushできないようにした。
  1. .gthub/CODEOWNERSに以下の設定し、mainにpushする。
     ```
     * @46nori
     ```
  2. GitHub settings/branchesでBranch Protection ruleを`main`で作成し、以下にチェックを入れておく。
    - [x] Require a pull request before merging
      - [x] Require approvals
      - [x] Dismiss stale pull request approvals when new commits are pushed
      - [x] Require review from Code Owners
- Dockerfileからz88dkのコンパイルを外した。結局、今のところ使っていないし、最近MACのDev Containerの起動時にエラーが出るようになってしまったので。必要になったら原因追及する。
- SD Cardに置くイメージファイル(IMAGE00.IMG)を、[cpm22-b.zip](http://www.cpm.z80.de/download/cpm22-b.zip)から生成するように変更した。
  - この中に入っているCPM.SYSは62K CP/Mなので、そのまま使用できる。`mkfs.cpm`コマンドでイメージファイルの予約トラックに書き込む。
  - それ以外のファイルは`cpmcp`コマンドでイメージファイルに埋め込む。

## 2023/10/07
- CP/Mの自律起動をサポートした。仕組みは以下。
  - AVRのEEPROMにCP/M BIOSを置き、起動時にSRAMにロードし、`BOOT:`を呼び出す。
  - `BOOT:`はmicroSD CardからCCP+BDOSを読み込んでCCPを起動するようにする。(`WBOOT:`でやることと同じ。)
- CP/M BIOSの変更
  - `BOOT:`でCCP+BDOSをmicroSD Cardから読み込むようにする。
- BIOSのINTEL HEXファイル生成
  - Makefileに`bios.ihx`ターゲットを追加。
- AVRの簡易モニタへの機能追加
  - EEPROMのダンプ機能(`de`コマンド)
  - SRAMからEEPROMへのSAVE機能(`esave`コマンド)
  - EEPROMからSRAMへのLOAD機能(`eload`コマンド) さらに外から使えるようにする。
- AVRのシステム起動時に、`eload`相当の関数を呼び出して、メモリにロードし、0x0000に`JP 0xf200`を書き込み、Z80にリセットをかける。
- 準備1 (EEPROMへのBIOS書き込み)
  1. `z80/cpm22`でmakeして`bios.ihx`を生成。
  2. `xload`コマンドで`bios.ihx`をXMODEM転送。0xf200にロードされる。
  3. `esave`コマンドでBIOSをEEPROMに書き込む。
    ```
    >xload
    Start XMODEM within 90s...
    
    Received 2816 bytes.
    >esave 0 $f200 2816
    >
    ```
- 準備2 (microSD Card)
  1. ~~FAT16~~FAT32でフォーマットする。
  2. `z80/cpm22/image`でmakeして生成される`DISK00.IMG`を、ルートディレクトリにコピーしておく。(00を01にするとB:ドライブとして認識される。以後、P:ドライブまで追加可能。)
- microSD Cardを入れて、本体をリセットするとCP/Mが起動するようになる。

## 2023/10/08
- [AVRの電圧が下がるとEEPROMのデータが壊れる問題](https://microchip.my.site.com/s/article/Prevent-EEPROM-corruption)があるらしい。([こちらも参考](https://gaje.jp/2022/05/09/4164/))
  - FUSEでBODを有効にし、LOW.BODLEVELを4.0Vでリセット状態になるように変更した。
  これにより現在のFUSEの設定は以下。
    - EXTENDED: 0xFF
    - HIGH: 0xC1
    - LOW: 0x0F  (旧 0xCF)
  - [仕様書](Hardware/Design.md#fuse-bits)も更新した。
- CP/Mの自律起動の改良
  - システムリセット時にDIPSW4がONならCP/Mの自動起動を、OFFなら簡易モニタのみ起動するようにした。
  - EEPROMに記録するのがBIOSのバイナリだけだと、ロード時にアドレスと長さが決め打ち実装になってしまう。ロードアドレスと長さ情報もEEPROMに記録できるよう、`esave2`コマンドを追加した。以下のフォーマットで記録される。  
    `esave2 <dst> <src> <length>`
    |アドレス|データ|
    |-------|------|
    |dst    |address|
    |dst+2  |length|
    |dst+4以降|srcから始まるlength分のデータ|
  - 以下が改良後の設定手順。
    ```
    >xload
    Start XMODEM within 90s...    ここでbios.ihxを転送

    Received 2816 bytes.
    >esave2 0 $f200 2816
    >de
    $0000 00 f2 00 0b c3 39 f3 c3 db f3 c3 7c f4 c3 83 f4  .....9.....|....
          ^^^^^ ^^^^^ エントリポイント, 長さ
    >

    ここでDIPSW1 ONにしてリセットボタンを押す

    === CP/M mode ===
    BIOS: 0xf200 - 0xfcff

    ATmega128 Tiny Monitor
    >
    ```
- z80/以下のMakefileのリファクタリングを行った。
- DIPSWでシステムの挙動を変更できるようにした。
  |DIP SW|機能     |OFF|ON |PORT|
  |:----:|--------|---|---|----|
  |  1   |CP/M起動|しない|する|PORTG3|
  |  2   |UART 0/1|9600bps|192000bps|PORTG4|
  |  3   ||||PORTE2|
  |  4   |外部SRAM wait|1 wait|2 wait|PORTE3|


さて、、これで最低限の必要なことはすべてやりつくしたかな。  
**本プロジェクトのサクセスクライテリアである　「CP/M-80が単独起動し、ZORK Iが遊べる環境の構築」　は達成されたといってよいだろう。**

今後は以下をちまちま行っていく予定。リファクタリングをどこまでやるかはモチベーション次第。
- TODO
  - AVR側
    - [手順書] ファームウェアのビルド、ターゲットへのダウンロード
    - [設計書] 簡易モニタの概要
    - [リファクタリング] F_CPUを再定義しているところがある
    - [リファクタリング] ASFへの無駄な依存の排除
    - [リファクタリング] pf_open()の高速化。DISK I/Oのボトルネックになっている
    - [リファクタリング] DISK WRITEが遅い。[Petit Fat FS](http://elm-chan.org/fsw/ff/00index_p.html)をやめて、ほかの方法を探す。
  - Z80側
    - [手順書] 簡易モニタからコンソール経由でINTEL HEXイメージをダウンロードさせて起動する方法
      - IPL+CCP/BDOS+BIOSのINTEL HEXの生成
    - [手順書] EEPROMにBIOSを書き込んで自動起動させる方法
      - BIOSのINTEL HEXの生成と、EEPROMへの書き込み方法
    - [設計書] BIOSの設計書
    - CP/Mイメージの概要
      - [手順書] イメージの生成方法
        - いろいろなイメージを作れるような環境も整備したほうがいいかも。
        - ZORKの取り込み方などはまだ自動化など。
      - [設計書] microSD CardでのDISKエミュレーションの仕組み
      - [設計書] イメージの構造
    - [リファクタリング] BIOSのDISK WITE高速化のためのブロッキング・デブロッキングサポート
  - システム全体
    - ~~[手順書] 基板、DIPSW、LEDの説明。必要な電源容量など。これは設計書か？~~ (2023/10/11)
    - [設計書] [ハード仕様書](Hardware/Design.md)の見直し
    - ~~トップのREADME.mdで、実現できたことの説明を追記~~ (2023/10/11)
    - ~~[手順書] PCB Rev1.0の改修方法~~ (2023/10/11)
    - PCBの作り直しはどうしようかなぁ

## 2023/10/10
- 不要なファイル削除、いろいろリファクタリングした。

## 2023/10/11
- [SetupGuide](./SetupGuide.md)を追加

## 2023/10/12
- `avr/src/main.c`でPORTB4をチェックし、CP/M modeでSD Cardの挿入状態を確認するようにした。
- z80/cpm以下のディレクトリ構成を変更し、`bios/`と`sys/`にファイルを分離した。

## 2023/10/14
- デモビデオを作成し、README.mdにリンクした。実体はYouTubeの[ここ](https://youtu.be/2_RJPk65XRE)にある。
- とりあえずここまでで一旦、開発終了。
- ~~コミット `1b330e465dc1f0a77b62a91e1d17c844d12498b4`に`v1.0`のタグを打った。~~(2023/10/22 でかいバグが見つかったので削除した)

## 2023/10/17
- CP/M BIOSに致命的なバグ発見。DPHは論理ディスクごとに必要なのにもかかわらず、A:ドライブ用しか用意していなかった。残メモリとの兼ね合いで、E:までの５台分の領域を確保した。0xf200-0xfe1fまで使用しているので、残メモリは481bytes。

## 2023/10/21
- CP/M BIOSのDISK WRITEのデブロッキング対応を行った。
  - AVRでの無駄なリードモディファイライトが減って、ライトが高速になった。
  - AVRへのライト要求を行う条件を以下とした。
    - BDOSがWRITEを呼ぶときCレジスタにセットするBlocking CodeがImmediate Writeのとき(C=1)。
    - BIOSの4セクタ分のデータバッファ(512byte)の最終ブロックに書き込まれたとき。
  - それ以外はなるべくSD Cardのクラスタサイズ(512byte)単位でライトできるよう、最大連続4セクタまでキャッシュする。
  - DSKSELとREADの前にPending Writeのフラグをチェックして書き出すようにした。Blocking Codeだけでは信用できないことがあったので。
  - ライトは高速化されたものの、SELDSKで呼ばれるPetit FatFSのpf_open()が処理時間の多くを占めている。Z80からの割り込みハンドラでブロッキング処理していることもあり、ここはもっと高速化させたいところ。しかしファイルが同時に１つしかopenできない仕様に制約されている。本システムの使い方は限定的なので、pf_open()のロジックに手を入れて、あらかじめ特定ファイルを複数同時オープンしておくように改造すればよいかもしれない。
- AVRのDISK WRITE(`em_disk_write()`)のバグを修正。
  - SDカードのクラスタ(512バイト境界)にアラインしてないデータをライトさせる場合、当該クラスタをリードモディファイライトする必要がある。ライトするデータ長が512バイト未満の場合が考慮されていなかった。このため未書き込みのデータ量の計算が０でなく、大量に残っている状態になってしまい、無駄な書き込みを行っていた。これがディレクトリを壊していた。

## 2023/10/22
- 簡易モニタ(ATmega128 Tiny Monitor)のコマンドリファレンスのドキュメントを追加。
- `develop`から`main`にマージし`v1.0`のタグを打ち直し。

## [続き](Diary3.md)