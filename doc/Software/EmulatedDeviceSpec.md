# Emulated external I/O device specification

## Port (English)
| Port | Direction | Function                              | Value                                |
| ---- | --------- | ------------------------------------- | ------------------------------------ |
| 0x00 | IN        | Input single character from Console   | Input character                      |
| 0x00 | OUT       | Undefined                             |                                      |
| 0x01 | IN        | Get Console Input Buffer status       | Unread characters count in Buffer    |
| 0x01 | OUT       | Undefined                             |                                      |
| 0x02 | IN        | Get Console Input Buffer size         | Buffer size                          |
| 0x02 | OUT       | Flush Console Input Buffer            |                                      |
| 0x03 | IN        | Get Console input interrupt level     | Interrupt level                      |
| 0x03 | OUT       | Set Console input interrupt level     | 0-127: Mode2 INT level<br>Other: No interrupt |
| 0x04 | IN        | Undefined                             |                                      |
| 0x04 | OUT       | Undefined                             |                                      |
| 0x05 | IN        | Undefined                             |                                      |
| 0x05 | OUT       | Output single character to Console    | Output character                     |
| 0x06 | IN        | Get Console Output Buffer status      | Remaining character count in Buffer  |
| 0x06 | OUT       | Undefined                             |                                      |
| 0x07 | IN        | Get Console output Buffer size        | Buffer size                          |
| 0x07 | OUT       | Flush Console Output Buffer           |                                      |
| 0x08 | IN        | Get Console Output interrupt level    | Interrupt level                      |
| 0x08 | OUT       | Set Console Output interrupt level    | 0-127: Mode2 INT level<br>Other: No interrupt |
| 0x09 | IN        | Undefined                             |                                      |
| 0x09 | OUT       | Undefined                             |                                      |
| 0x0a | IN        | Get selected DISK                     | Currently selected DISK number       |
| 0x0a | OUT       | Get Selected DISK status              | 0: Success / 1: Error                |
| 0x0b | IN        | Get DISK WRITE Position               | Physical WRITE position and reset sequencer |
| 0x0b | OUT       | Set DISK WRITE Position               | Physical WRITE position              |
| 0x0c | IN        | Get DISK WRITE Buffer address         | Buffer address and reset sequencer   |
| 0x0c | OUT       | Set DISK WRITE Buffer address         | Buffer address                       |
| 0x0d | IN        | Get DISK WRITE Buffer length          | Data length and reset sequencer      |
| 0x0d | OUT       | Set DISK WRITE Buffer length          | Data length                          |
| 0x0e | IN        | Get DISK WRITE status                 | Bit 0: Completed(0) / Writing(1)<br>Bit 1: Success(0) / Error(1) |
| 0x0e | OUT       | Execute DISK WRITE                    |                                      |
| 0x0f | IN        | Get DISK WRITE interrupt level        | Interrupt level                      |
| 0x0f | OUT       | Set DISK WRITE interrupt level        | 0-127: Mode2 INT level<br>Other: No interrupt |
| 0x10 | IN        | Get DISK WRITE Position               | Physical READ position and reset sequencer |
| 0x10 | OUT       | Set DISK WRITE Position               | Physical READ position               |
| 0x11 | IN        | Get DISK READ Buffer address          | Buffer addreess and reset sequencer  |
| 0x11 | OUT       | Set DISK READ Buffer address          | Buffer addreess                      |
| 0x12 | IN        | Get DISK READ Buffer length           | Data length and reset sequencer      |
| 0x12 | OUT       | Set DISK READ Buffer length           | Data length                          |
| 0x13 | IN        | Get DISK READ status                  | Bit 0: Completed(0) / Reading(1)<br>Bit 1: Success(0) / Error(1) |
| 0x13 | OUT       | Execute DISK READ                     | 0-127: Mode2 INT level<br>Other: No interrupt |
| 0x14 | IN        | Get DISK READ interrupt level         | Interrupt level                      |
| 0x14 | OUT       | Set DISK READ interrupt level         | 0-127: Mode2 INT level<br>Other: No interrupt |
| 0x15 | IN        | Undefined                             |                                      |
| 0x15 | OUT       | Undefined                             |                                      |
| 0x16 | IN        | Undefined                             |                                      |
| 0x16 | OUT       | Undefined                             |                                      |
| 0x17 | IN        | Undefined                             |                                      |
| 0x17 | OUT       | Undefined                             |                                      |
| 0x18 | IN        | Undefined                             |                                      |
| 0x18 | OUT       | Undefined                             |                                      |
| 0x19 | IN        | Undefined                             |                                      |
| 0x19 | OUT       | Undefined                             |                                      |
| 0x1a | IN        | Undefined                             |                                      |
| 0x1a | OUT       | Undefined                             |                                      |
| 0x1b | IN        | Undefined                             |                                      |
| 0x1b | OUT       | Undefined                             |                                      |
| 0x1c | IN        | Undefined                             |                                      |
| 0x1c | OUT       | Undefined                             |                                      |
| 0x1d | IN        | Get interrupt level for resumption from the breakpoint | Interrupt level                               |
| 0x1d | OUT       | Set interrupt level for resumption from the breakpoint | 0-127: Mode2 INT level<br>Other: No interrupt |
| 0x1e | IN        | Reset sequencer                       | Reset sequencer and wait 1st byte |
| 0x1e | OUT       | Send debug info                       | Send debug info(breakpoint, registers and stack data) |
| 0x1f | IN        | Get LED Status                        | bit 0: LED0 OFF(0) / ON(1)<br>bit 1: LED1 OFF(0) / ON(1)<br>bit 2: LED2 OFF(0) / ON(1) |
| 0x1f | OUT       | Set LED                               | bit 0: LED0 OFF(0) / ON(1)<br>bit 1: LED1 OFF(0) / ON(1)<br>bit 2: LED2 OFF(0) / ON(1) |

From 0x20 onwards, it becomes a shadow area for 0x00-0x1f.

---
## Port (Japanese)
|Port|Direction|Function|Value|
|:----|:----|:----|:----|
|0x00|IN |コンソール1文字入力|入力文字|
|0x00|OUT|未定義||
|0x01|IN |コンソール入力バッファステータス取得|未入力文字数|
|0x01|OUT|未定義||
|0x02|IN |コンソールバッファサイズ取得|バッファサイズ|
|0x02|OUT|コンソール入力バッファフラッシュ||
|0x03|IN |コンソール入力割り込みレベル取得|割り込みレベル|
|0x03|OUT|コンソール入力割り込みレベル設定|0-127: Mode2割り込みレベル<br>それ以外: 割り込み禁止|
|0x04|IN |未定義||
|0x04|OUT|未定義||
|0x05|IN |未定義||
|0x05|OUT|コンソール1文字出力|出力文字|
|0x06|IN |コンソール出力バッファステータス取得|未出力文字数|
|0x06|OUT|未定義||
|0x07|IN |コンソール出力バッファサイズ取得|バッファサイズ|
|0x07|OUT|コンソール出力バッファフラッシュ||
|0x08|IN |コンソール出力エンプティ割り込みレベル設定|0-127: Mode2割り込みレベル<br>それ以外: 割り込み禁止|
|0x08|OUT|コンソール出力エンプティ割り込みレベル取得|割り込みレベル|
|0x09|IN |未定義||
|0x09|OUT|未定義||
|0x0a|IN|DISK選択ステータス|成功(0) / エラー(1)|
|0x0a|OUT|DISK選択|0-15: DISK番号|
|0x0b|IN|DISKライト物理位置参照・シーケンサーリセット|ライト位置|
|0x0b|OUT|DISKライト物理位置指定|ライト位置|
|0x0c|IN|DISKライトバッファアドレス参照・シーケンサーリセット|バッファアドレス|
|0x0c|OUT|DISKライトバッファアドレス設定|バッファアドレス|
|0x0d|IN|DISKライトバッファ長参照・シーケンサーリセット|長さ|
|0x0d|OUT|DISKライトバッファ長設定|長さ|
|0x0e|IN|DISKライトステータス|Bit0: 完了(0) / ライト中(1)<br>Bit1: 成功(0) / エラー(1)|
|0x0e|OUT|DISKライト実行||
|0x0f|IN |DISKライト完了割り込みレベル設定|0-127: Mode2割り込みレベル<br>それ以外: 割り込み禁止|
|0x0f|OUT|DISKライト完了割り込みレベル取得|割り込みレベル|
|0x10|IN|DISKリード物理位置参照・シーケンサーリセット|リード位置|
|0x10|OUT|DISKリード物理位置指定|リード位置|
|0x11|IN|DISKリードバッファアドレス参照・シーケンサーリセット|バッファアドレス|
|0x11|OUT|DISKリードバッファアドレス設定|バッファアドレス|
|0x12|IN|DISKリードバッファ長参照・シーケンサーリセット|長さ|
|0x12|OUT|DISKリードバッファ長設定|長さ|
|0x13|IN|DISKリードステータス|Bit0: 完了(0) / リード中(1)<br>Bit1: 成功(0) / エラー(1)|
|0x13|OUT|DISKリード実行||
|0x14|IN |DISKリード完了割り込みレベル取得|割り込みレベル|
|0x14|OUT|DISKリード完了割り込みレベル設定|0-127: Mode2割り込みレベル<br>それ以外: 割り込み禁止|
|0x15|IN |未定義||
|0x15|OUT|未定義||
|0x16|IN |未定義||
|0x16|OUT|未定義||
|0x17|IN |未定義||
|0x17|OUT|未定義||
|0x18|IN |未定義||
|0x18|OUT|未定義||
|0x19|IN |未定義||
|0x19|OUT|未定義||
|0x1a|IN |未定義||
|0x1a|OUT|未定義||
|0x1b|IN |未定義||
|0x1b|OUT|未定義||
|0x1c|IN |未定義||
|0x1c|OUT|未定義||
|0x1d|IN|ブレークポイント復帰割り込みレベル取得|割り込みレベル|
|0x1d|OUT|ブレークポイント復帰割り込みレベル設定|0-127: Mode2割り込みレベル<br>それ以外: 割り込み禁止|
|0x1e|IN|デバッグ情報通知シーケンスリセット|シーケンサーをリセットする。|
|0x1e|OUT|デバッグ情報通知|デバッグ情報(ブレークアドレス/Z80レジスタ/スタックデータ)の通知。デバッグ領域の値を逆順に送る。|
|0x1f|IN |LED点灯状態取得| bit 0: LED0 OFF(0) / ON(1)<br>bit 1: LED1 OFF(0) / ON(1)<br>bit 2: LED2 OFF(0) / ON(1) |
|0x1f|OUT|LED点灯設定| bit 0: LED0 OFF(0) / ON(1)<br>bit 1: LED1 OFF(0) / ON(1)<br>bit 2: LED2 OFF(0) / ON(1) |

- 0x20以降は0x00-0x1fのシャドウエリアとなる。
- 複数バイトが設定できるPORTはシーケンサーを持つ。値をセットする前にシーケンサーをリセットすること。

## Console I/O
### Console Input
AVRはUART1の入力をリングバッファに値を保存しており、Z80からの1文字入力要求があると、このバッファから値を返す。
割り込み設定を行うことで、リングバッファに文字がキューイングされると同時にZ80に割り込みをかけることができる。

- PORT 0x00 コンソール1文字入力
  - **[IN]** バッファリングされている文字を返す。バッファが空の場合は0x00を返す。
- PORT 0x01 コンソール入力バッファステータス取得
  - **[IN]** 入力バッファに残っている文字数を返す。
- PORT 0x02 コンソールバッファサイズ取得
  - **[IN]** 入力バッファのサイズを返す。
- PORT 0x02 コンソール入力バッファフラッシュ
  - **[OUT]** 入力バッファを空にする。
- PORT 0x03 コンソール入力割り込み設定
  - **[OUT]** コンソール入力発生時にZ80にMode2割り込みをかけることができる。0-127を設定するとそのレベルでの割り込みが発生する。128-255に設定すると割り込みは発生しない。デフォルト値は128。
  - **[IN]** 現在の割り込みレベルの値を返す。

### Console Output
- PORT 0x05 コンソール1文字出力
  - **[OUT]** 文字する出力をセットする。
- PORT 0x06 コンソール出力バッファステータス取得
  - **[IN]** まだ出力されずバッファに残っている文字数を返す。
- PORT 0x07
  - **[IN]** 出力バッファのサイズを返す。
  - **[OUT]** 出力バッファを空にする。
- PORT 0x08 コンソール出力割り込み設定
  - **[OUT]** 出力バッファが以下の状態になった時にZ80にMode2割り込みをかけることができる。0-127を設定するとそのレベルでの割り込みが発生する。128-255に設定すると割り込みは発生しない。デフォルト値は128。
    - 未出力データ数が出力バッファサイズの1/2に達した
    - 未出力データ数が出力バッファサイズの1/4に達した
    - 未出力データ数が0になった
  - **[IN]** 現在の割り込みレベルの値を返す。

## DISK I/O
### DISK選択
- PORT 0x0a DISK選択
  - **[IN]** DISK選択結果　成功(0) / エラー(1)
  - **[OUT]** DISK番号(0-15)を選択しアクセス可能にする。指定したDISK番号のイメージファイルDISKxx.IMG (xxはDISK番号)があらかじめ存在していること。
### DISK WRITE
- PORT 0x0b
  - **[IN]** DISKライト位置参照・シーケンサーリセット
  - **[OUT]** DISKライト位置を指定する。Highアドレスの値から4バイト書き込む。
- PORT 0x0c
  - **[IN]** DISKライトバッファアドレス参照・シーケンサーリセット
  - **[OUT]** DISKライトバッファアドレスを指定する。Highアドレスから2バイト書き込む。
- PORT 0x0d
  - **[IN]** DISKライトバッファ長参照・シーケンサーリセット
  - **[OUT]** DISKライトバッファ長を指定する。Highアドレスから2バイト書き込む。
- PORT 0x0e
  - **[OUT]** DISKライト実行
  - **[IN]** DISKライト状態を返す。
    - Bit0: 完了(0) / 実行中(1)
    - Bit1: 成功(0) / エラー(1)
- PORT 0x0f　DISKライト完了割り込み設定
  - **[OUT]** DISKライト完了時にZ80にMode2割り込みをかけることができる。0-127を設定するとそのレベルでの割り込みが発生する。128-255に設定すると割り込みは発生しない。デフォルト値は128。
### DISK READ
- PORT 0x10
  - **[OUT]** DISKリード位置を指定する。Highアドレスから4バイト書き込む。
  - **[IN]** DISKリード位置参照・シーケンサーリセット
- PORT 0x11
  - **[OUT]** DISKリードバッファアドレスを指定する。Highアドレスから2バイト書き込む。
  - **[IN]** DISKリードバッファアドレス参照・シーケンサーリセット
- PORT 0x12
  - **[OUT]** DISKリードバッファ長を指定する。Highアドレスから2バイト書き込む。
  - **[IN]** DISKリードバッファ長参照・シーケンサーリセット
- PORT 0x13
  - **[OUT]** DISKリード実行
  - **[IN]** DISKリード状態を返す。
    - Bit0: 完了(0) / 実行中(1)
    - Bit1: 成功(0) / エラー(1)
- PORT 0x14 DISKリード完了割り込み設定
  - **[OUT]** DISKリード完了時にZ80にMode2割り込みをかけることができる。0-127を設定するとそのレベルでの割り込みが発生する。128-255に設定すると割り込みは発生しない。デフォルト値は128。
  - **[IN]** 現在の割り込みレベルの値を返す。

DISK READ/WRITEの起動はOUT命令の割り込みハンドラで受理される。その起動イベントをREQとする。この割り込みハンドラ内では外部SRAMへのアクセスができないため、SD Cardのアクセスは実行しない。(外部SRAMのアクセスには/BUSREQを有効にする必要があるが、割り込みサイクルでの処理は両立できない。) SD CardへのアクセスおよびSRAMとのデータ転送は、10msの周期割り込みハンドラ内で行う。ここではREAD/WRITE用のステートマシンが順番に実行される。READとWRITEは同時に実行されることはない。

rd.DOINGおよびwr.DOINGでSD CardのアクセスとSRAMとのデータ転送が行われる。エラーでも正常終了でもIDLEに戻る(DONEイベント)。
リードライト要求は、IDLEおよびREJECTED状態で受理される。
- イベント
    |Event| READ          | WRITE          |
    |-----|---------------|----------------|
    | REQ | READ実行(0x13) | WRITE実行(0x0e)|
    | DONE| READ完了       | WRITE完了      |
    | INT | 周期割り込み    | 周期割り込み     |

- 状態
    |State     | READ                    | WRITE 　　　　　　　　　　　|
    |----------|-------------------------|-------------------------|
    |IDLE      | READ要求待ち。           | WRITE要求待ち。            |
    |REQUESTING| READ実行待ち。           | WRITE実行待ち。            |
    |DOING     | READ実行中。             | WRITE実行中。             |
    |REJECTED  | 要求拒否。次のREAD要求待ち。| 要求拒否。次のWRITE要求待ち。|

- ステートチャート
    ```mermaid
    stateDiagram-v2
        state READ {
            [*]            --> rd.IDLE
            rd.IDLE　      --> rd.REQUESTING: REQ
            rd.IDLE　      --> rd.IDLE: INT
            rd.REQUESTING  --> rd.DOING : INT
            rd.REQUESTING  --> rd.REJECTED : REQ
            rd.REQUESTING  --> rd.REQUESTING : INT
            rd.DOING       --> rd.REJECTED : REQ
            rd.DOING       --> rd.DOING : INT
            rd.DOING       --> rd.IDLE : DONE
            rd.REJECTED    --> rd.REJECTED : INT
            rd.REJECTED    --> rd.DOING : REQ
        }

        state WRITE {
            [*]         --> wr.IDLE
            wr.IDLE        --> wr.REQUESTING: REQ
            wr.IDLE　      --> wr.IDLE: INT
            wr.REQUESTING  --> wr.DOING : INT
            wr.REQUESTING  --> wr.REJECTED : REQ
            wr.REQUESTING  --> wr.REQUESTING : INT
            wr.DOING       --> wr.REJECTED : REQ
            wr.DOING       --> wr.DOING : INT
            wr.DOING       --> wr.IDLE : DONE
            wr.REJECTED    --> wr.REJECTED : INT
            wr.REJECTED    --> wr.DOING : REQ
        }
    ```

## Debug
## LED
