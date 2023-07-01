# Emulated external I/O device specification

| Port | Direction | Function                              | Value                                |
| ---- | --------- | ------------------------------------- | ------------------------------------ |
| 0x00 | OUT       | Undefined                             |                                      |
| 0x00 | IN        | Undefined                             |                                      |
| 0x01 | OUT       | Set Console input interrupt           | 0-127: Mode2 INT vector<br>Other: No interrupt |
| 0x01 | IN        | Get Console input interrupt seting    | Console input interrupt seting       |
| 0x02 | OUT       | Output single character to Console    | Output character                     |
| 0x02 | IN        | Input single character from Console   | Input character                      |
| 0x03 | OUT       | Flush Console Input Buffer            | N/A                                  |
| 0x03 | IN        | Get Console Input Buffer status       | Remaining character count in buffer  |
| 0x04 | OUT       | Select DISK                           | 0-15: DISK number                    |
| 0x04 | IN        | Get selected DISK                     | Currently selected DISK number       |
| 0x05 | OUT       | Set DISK WRITE Position (low)         | Low byte of the physical position    |
| 0x05 | IN        | Get DISK WRITE Position (low)         | Low byte of the physical position    |
| 0x06 | OUT       | Set DISK WRITE Position (middle)      | Middle byte of the physical position |
| 0x06 | IN        | Get DISK WRITE Position (middle)      | Middle byte of the physical position |
| 0x07 | OUT       | Set DISK WRITE Position (high)        | High byte of the physical position   |
| 0x07 | IN        | Get DISK WRITE Position (high)        | High byte of the physical position   |
| 0x08 | OUT       | Set DISK WRITE Buffer address (low)   | Low address of the buffer            |
| 0x08 | IN        | Get DISK WRITE Buffer address (low)   | Low address of the buffer            |
| 0x09 | OUT       | Set DISK WRITE Buffer address (high)  | High address of the buffer           |
| 0x09 | IN        | Get DISK WRITE Buffer address (high)  | High address of the buffer           |
| 0x0a | OUT       | Set DISK WRITE Buffer length (low)    | Low byte of the length               |
| 0x0a | IN        | Get DISK WRITE Buffer length (low)    | Low byte of the length               |
| 0x0b | OUT       | Set DISK WRITE Buffer length (high)   | High byte of the length              |
| 0x0b | IN        | Get DISK WRITE Buffer length (high)   | High byte of the length              |
| 0x0c | OUT       | Execute DISK WRITE                    | 0-127: Mode2 INT vector<br>Other: No interrupt |
| 0x0c | IN        | Get DISK WRITE status                 | Bit0: Completed(0) / Writing(1)<br>Bit1: Success(0) / Error(1) |
| 0x0d | OUT       | Set DISK WRITE Position (low)         | Low byte of the physical position    |
| 0x0d | IN        | Get DISK WRITE Position (low)         | Low byte of the physical position    |
| 0x0e | OUT       | Set DISK WRITE Position (middle)      | Middle byte of the physical position |
| 0x0e | IN        | Get DISK WRITE Position (middle)      | Middle byte of the physical position |
| 0x0f | OUT       | Set DISK WRITE Position (high)        | High byte of the physical position   |
| 0x0f | IN        | Get DISK WRITE Position (high)        | High byte of the physical position   |
| 0x10 | OUT       | Set DISK READ Buffer address (low)    | Low address of the buffer            |
| 0x10 | IN        | Get DISK READ Buffer address (low)    | Low address of the buffer            |
| 0x11 | OUT       | Set DISK READ Buffer address (high)   | High address of the buffer           |
| 0x11 | IN        | Get DISK READ Buffer address (high)   | High address of the buffer           |
| 0x12 | OUT       | Set DISK READ Buffer length (low)     | Low byte of the length               |
| 0x12 | IN        | Get DISK READ Buffer length (low)     | Low byte of the length               |
| 0x13 | OUT       | Set DISK READ Buffer length (high)    | High byte of the length              |
| 0x13 | IN        | Get DISK READ Buffer length (high)    | High byte of the length              |
| 0x14 | OUT       | Execute DISK READ                     | 0-127: Mode2 INT vector<br>Other: No interrupt |
| 0x14 | IN        | Get DISK READ status                  | Bit0: Completed(0) / Reading(1)<br>Bit1: Success(0) / Error(1) |

From 0x15 onwards, it becomes a shadow area for 0x00-0x14.

----
**Japanese**
|Port|Direction|Function|Value|
|:----|:----|:----|:----|
|0x00|OUT|未定義||
|0x00|IN|未定義||
|0x01|OUT|コンソール入力割り込み設定|0-127: Mode2割り込みベクタ番号<br>それ以外: 割り込みなし|
|0x01|IN|コンソール入力割り込み設定参照|↑|
|0x02|OUT|コンソール1文字出力|出力文字|
|0x02|IN|コンソール1文字入力|入力文字|
|0x03|OUT|コンソール入力バッファフラッシュ|N/A|
|0x03|IN|コンソール入力バッファステータス|残文字数|
|0x04|OUT|DISK選択|0-15: DISK番号|
|0x04|IN|DISK番号参照|選択中のDISK番号|
|0x05|OUT|DISKライト物理位置指定|ライト位置の下位アドレス|
|0x05|IN|DISKライト物理位置指定|ライト位置の下位アドレス|
|0x06|OUT|DISKライト物理位置指定|ライト位置の中位アドレス|
|0x06|IN|DISKライト物理位置指定|ライト位置の中位アドレス|
|0x07|OUT|DISKライト物理位置指定|ライト位置の上位アドレス|
|0x07|IN|DISKライト物理位置指定|ライト位置の上位アドレス|
|0x08|OUT|DISKライトバッファアドレス設定|バッファの下位アドレス|
|0x08|IN|DISKライトバッファアドレス参照|バッファの下位アドレス|
|0x09|OUT|DISKライトバッファアドレス設定|バッファの上位アドレス|
|0x09|IN|DISKライトバッファアドレス参照|バッファの上位アドレス|
|0x0a|OUT|DISKライトバッファ長設定|長さの下位バイト|
|0x0a|IN|DISKライトバッファ長参照|長さの下位バイト|
|0x0b|OUT|DISKライトバッファ長設定|長さの上位バイト|
|0x0b|IN|DISKライトバッファ長参照|長さの上位バイト|
|0x0c|OUT|DISKライト実行|0-127: Mode2割り込みベクタ番号<br>それ以外: 割り込みなし|
|0x0c|IN|DISKライトステータス|Bit0: 完了(0) / リード中(1)<br>Bit1: 成功(0) / エラー(1)|
|0x0d|OUT|DISKリード物理位置指定|リード位置の下位アドレス|
|0x0d|IN|DISKリード物理位置指定|リード位置の下位アドレス|
|0x0e|OUT|DISKリード物理位置指定|リード位置の中位アドレス|
|0x0e|IN|DISKリード物理位置指定|リード位置の中位アドレス|
|0x0f|OUT|DISKリード物理位置指定|リード位置の上位アドレス|
|0x0f|IN|DISKリード物理位置指定|リード位置の上位アドレス|
|0x10|OUT|DISKリードバッファアドレス設定|バッファの下位アドレス|
|0x10|IN|DISKリードバッファアドレス参照|バッファの下位アドレス|
|0x11|OUT|DISKリードバッファアドレス設定|バッファの上位アドレス|
|0x11|IN|DISKリードバッファアドレス参照|バッファの上位アドレス|
|0x12|OUT|DISKリードバッファ長設定|長さの下位バイト|
|0x12|IN|DISKリードバッファ長参照|長さの下位バイト|
|0x13|OUT|DISKリードバッファ長設定|長さの上位バイト|
|0x13|IN|DISKリードバッファ長参照|長さの上位バイト|
|0x14|OUT|DISKリード実行|0-127: Mode2割り込みベクタ番号<br>それ以外: 割り込みなし|
|0x14|IN|DISKリードステータス|Bit0: 完了(0) / リード中(1)<br>Bit1: 成功(0) / エラー(1)|

0x15以降は0x00-0x14のシャドウエリアとなる。