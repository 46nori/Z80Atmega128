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
  - The local labels `/WR`, `/RD` for ATmega128 conflicted with the Z80's `/WR`, `/RD`, causing unintended connections on the PCB, so the label name was changed as shown below.
    | PIN | Rev1.00| Rev2.00 |
    |:---:|--------|---------|
    |  33 |  /WR   | /AVR_WR |
    |  34 |  /RD   | /AVR_RD |
    |  43 |  ALE   | AVR_ALE |
- ExtIO_sch.kicad_sch
  - Pull up with 10K the input of 74HC32 for driving LED1/2/3.
  - Pull up with 10K the input of 74HC04 and 74HC125 for driving  LED /HALT and /BUSACK.
  - Fixed input being unstable at reset or when ATmega128 firmware is not running

### PCB
- Changed the position of R1(10K), because there are no space between the 6-pin box header of AVRICP and R1.
- Modified the land pitch of POWER Switch (2-circuit 3-contact toggle switch).
- Changed the footprint of USB serial converter board (AE-CH340E-TPPEC) and reversed the pin assignment.
- Errata
  - The silk printing of U18 and U19 are incorrect.
    - [Incorrect]
      ```
       o   o   o   o   o   o
          RX  TX  5V      GND
      ```
    - [Correct]
      ```
       o   o   o   o   o   o
      GND     5V  TX  RX
      ```

