# Hardware Release Note
----
## Rev 1.00
- Initial Release
- Errata
  - [Front Plane](./PCB/PCB1.0-FP-Errata.pdf)
    - Pattern cut 7
  - [Back Plane](./PCB/PCB1.0-BP-Errata.pdf)
    - Pattern cut 1
    - Patch 6

----
## Rev 2.00
### Schematics
- Z80ATmega128.kicad_sch
  - ATmega128に設定したラベルが、Z80のラベル`/WR`,`/RD`と衝突しており、PCB上で意図しない接続が発生していたため、以下のようにラベル名を変更。
    | PIN | Rev1.00| Rev2.00 |
    |:---:|--------|---------|
    |  33 |  /WR   | /AVR_WR |
    |  34 |  /RD   | /AVR_RD |
    |  43 |  ALE   | AVR_ALE |
- ExtIO_sch.kicad_sch
  - LED1/2/3のドライブ用素子(74HC32)の入力を10Kでプルアップ
  - /HALT,/BUSACKのドライブ用素子(74HC04, 74HC125)の入力を10Kでプルアップ
  - リセット時やATmega128のファームウェアが動作していないときの入力が不安定になるのを修正

### PCB
- R1(10K)の位置を変更。AVRICPの6pinボックスヘッダとのクリアランスが取れていなかったため。
- POWER Switch(2回路3接点トグルスイッチ)のランドピッチが間違っていたのを修正。
- USBシリアル変換基板(AE-CH340E-TYPEC)のFootprintを変更し、ピン配置を逆転させた。Rev1.00では変換基板を裏返しに設置していた。
- Errata
  - U18, U19のUARTのシルクが間違っている。
    - [誤]
      ```
       o   o   o   o   o   o
          RX  TX  5V      GND
      ```
    - [正]
      ```
       o   o   o   o   o   o
      GND     5V  TX  RX
      ```

----
## Rev 2.10
### PCB
- Rev 2.00のErrataの修正
- DIP SWの説明
  - SW3をSD CardのWrite Protectに割り当て
- 一部シルクを白抜き表示に変更