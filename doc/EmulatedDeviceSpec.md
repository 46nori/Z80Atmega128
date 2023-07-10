# Emulated external I/O device specification

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
| 0x0a | OUT       | Select DISK                           | 0-15: DISK number                    |
| 0x0b | IN        | Get DISK WRITE Position (low)         | Low byte of the physical position    |
| 0x0b | OUT       | Set DISK WRITE Position (low)         | Low byte of the physical position    |
| 0x0c | IN        | Get DISK WRITE Position (middle)      | Middle byte of the physical position |
| 0x0c | OUT       | Set DISK WRITE Position (middle)      | Middle byte of the physical position |
| 0x0d | IN        | Get DISK WRITE Position (high)        | High byte of the physical position   |
| 0x0d | OUT       | Set DISK WRITE Position (high)        | High byte of the physical position   |
| 0x0e | IN        | Get DISK WRITE Buffer address (low)   | Low address of the buffer            |
| 0x0e | OUT       | Set DISK WRITE Buffer address (low)   | Low address of the buffer            |
| 0x0f | IN        | Get DISK WRITE Buffer address (high)  | High address of the buffer           |
| 0x0f | OUT       | Set DISK WRITE Buffer address (high)  | High address of the buffer           |
| 0x10 | IN        | Get DISK WRITE Buffer length (low)    | Low byte of the length               |
| 0x10 | OUT       | Set DISK WRITE Buffer length (low)    | Low byte of the length               |
| 0x11 | IN        | Get DISK WRITE Buffer length (high)   | High byte of the length              |
| 0x11 | OUT       | Set DISK WRITE Buffer length (high)   | High byte of the length              |
| 0x12 | IN        | Get DISK WRITE status                 | Bit 0: Completed(0) / Writing(1)<br>Bit 1: Success(0) / Error(1) |
| 0x12 | OUT       | Execute DISK WRITE                    |                                      |
| 0x13 | IN        | Get DISK WRITE interrupt level        | Interrupt level                      |
| 0x13 | OUT       | Set DISK WRITE interrupt level        | 0-127: Mode2 INT level<br>Other: No interrupt |
| 0x14 | IN        | Get DISK WRITE Position (low)         | Low byte of the physical position    |
| 0x14 | OUT       | Set DISK WRITE Position (low)         | Low byte of the physical position    |
| 0x15 | IN        | Get DISK WRITE Position (middle)      | Middle byte of the physical position |
| 0x15 | OUT       | Set DISK WRITE Position (middle)      | Middle byte of the physical position |
| 0x16 | IN        | Get DISK WRITE Position (high)        | High byte of the physical position   |
| 0x16 | OUT       | Set DISK WRITE Position (high)        | High byte of the physical position   |
| 0x17 | IN        | Get DISK READ Buffer address (low)    | Low address of the buffer            |
| 0x17 | OUT       | Set DISK READ Buffer address (low)    | Low address of the buffer            |
| 0x18 | IN        | Get DISK READ Buffer address (high)   | High address of the buffer           |
| 0x18 | OUT       | Set DISK READ Buffer address (high)   | High address of the buffer           |
| 0x19 | IN        | Get DISK READ Buffer length (low)     | Low byte of the length               |
| 0x19 | OUT       | Set DISK READ Buffer length (low)     | Low byte of the length               |
| 0x1a | IN        | Get DISK READ Buffer length (high)    | High byte of the length              |
| 0x1a | OUT       | Set DISK READ Buffer length (high)    | High byte of the length              |
| 0x1b | IN        | Get DISK READ status                  | Bit 0: Completed(0) / Reading(1)<br>Bit 1: Success(0) / Error(1) |
| 0x1b | OUT       | Execute DISK READ                     | 0-127: Mode2 INT level<br>Other: No interrupt |
| 0x1c | IN        | Get DISK READ interrupt level         | Interrupt level                      |
| 0x1c | OUT       | Set DISK READ interrupt level         | 0-127: Mode2 INT level<br>Other: No interrupt |
|  ..  | IN        | Undefined                             |                                      |
|  ..  | OUT       | Undefined                             |                                      |
| 0x1f | IN        | Get LED Status                        | bit 0: LED0 OFF(0) / ON(1)<br>bit 1: LED1 OFF(0) / ON(1)<br>bit 2: LED2 OFF(0) / ON(1) |
| 0x1f | OUT       | Set LED                               | bit 0: LED0 OFF(0) / ON(1)<br>bit 1: LED1 OFF(0) / ON(1)<br>bit 2: LED2 OFF(0) / ON(1) |

From 0x20 onwards, it becomes a shadow area for 0x00-0x1f.

----
**Japanese**
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
|0x0a|IN|DISK番号参照|選択中のDISK番号|
|0x0a|OUT|DISK選択|0-15: DISK番号|
|0x0b|IN|DISKライト物理位置指定|ライト位置の下位アドレス|
|0x0b|OUT|DISKライト物理位置指定|ライト位置の下位アドレス|
|0x0c|IN|DISKライト物理位置指定|ライト位置の中位アドレス|
|0x0c|OUT|DISKライト物理位置指定|ライト位置の中位アドレス|
|0x0d|IN|DISKライト物理位置指定|ライト位置の上位アドレス|
|0x0d|OUT|DISKライト物理位置指定|ライト位置の上位アドレス|
|0x0e|IN|DISKライトバッファアドレス参照|バッファの上位アドレス|
|0x0e|OUT|DISKライトバッファアドレス参照|バッファの下位アドレス|
|0x0f|IN|DISKライトバッファアドレス参照|バッファの上位アドレス|
|0x0f|OUT|DISKライトバッファアドレス設定|バッファの上位アドレス|
|0x10|IN|DISKライトバッファ長参照|長さの下位バイト|
|0x10|OUT|DISKライトバッファ長設定|長さの下位バイト|
|0x11|IN|DISKライトバッファ長参照|長さの上位バイト|
|0x11|OUT|DISKライトバッファ長設定|長さの上位バイト|
|0x12|IN|DISKライトステータス|Bit0: 完了(0) / リード中(1)<br>Bit1: 成功(0) / エラー(1)|
|0x12|OUT|DISKライト実行||
|0x13|IN |DISKライト完了割り込みレベル設定|0-127: Mode2割り込みレベル<br>それ以外: 割り込み禁止|
|0x13|OUT|DISKライト完了割り込みレベル取得|割り込みレベル|
|0x14|IN|DISKリード物理位置指定|リード位置の下位アドレス|
|0x14|OUT|DISKリード物理位置指定|リード位置の下位アドレス|
|0x15|IN|DISKリード物理位置指定|リード位置の中位アドレス|
|0x15|OUT|DISKリード物理位置指定|リード位置の中位アドレス|
|0x16|IN|DISKリード物理位置指定|リード位置の上位アドレス|
|0x16|OUT|DISKリード物理位置指定|リード位置の上位アドレス|
|0x17|IN|DISKリードバッファアドレス参照|バッファの下位アドレス|
|0x17|OUT|DISKリードバッファアドレス設定|バッファの下位アドレス|
|0x18|IN|DISKリードバッファアドレス参照|バッファの上位アドレス|
|0x18|OUT|DISKリードバッファアドレス設定|バッファの上位アドレス|
|0x19|IN|DISKリードバッファ長参照|長さの下位バイト|
|0x19|OUT|DISKリードバッファ長設定|長さの下位バイト|
|0x1a|IN|DISKリードバッファ長参照|長さの上位バイト|
|0x1a|OUT|DISKリードバッファ長設定|長さの上位バイト|
|0x1b|IN|DISKリードステータス|Bit0: 完了(0) / リード中(1)<br>Bit1: 成功(0) / エラー(1)|
|0x1b|OUT|DISKリード実行||
|0x1c|IN |DISKリード完了割り込みレベル設定|0-127: Mode2割り込みレベル<br>それ以外: 割り込み禁止|
|0x1c|OUT|DISKリード完了割り込みレベル取得|割り込みレベル|
| .. |IN|未定義|
| .. |OUT|未定義|
|0x1f|IN |LED点灯状態取得| bit 0: LED0 OFF(0) / ON(1)<br>bit 1: LED1 OFF(0) / ON(1)<br>bit 2: LED2 OFF(0) / ON(1) |
|0x1f|OUT|LED点灯設定| bit 0: LED0 OFF(0) / ON(1)<br>bit 1: LED1 OFF(0) / ON(1)<br>bit 2: LED2 OFF(0) / ON(1) |

0x20以降は0x00-0x1fのシャドウエリアとなる。