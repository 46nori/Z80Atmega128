# How to make CP/M DISK Image

## Overview
This system emulates a CP/M disk on a microSD Card. Disk images are managed as files on the microSD Card, so if multiple image files are in it, they are considered independent disk drives.

The microSD Card must be formatted with **FAT32**.

Disk image file naming conventions
  - `DISKnn.IMG` (`nn` is 00-15, corresponding to drives A:-P:)

`DISK00.IMG`, which is drive A: must contain the CP/M system (CCP+BDOS). Due to memory limitations, the BIOS on this system only supports up to 5 drives. Therefore, the actual usable drives are up to 00-04.

The disk image must match the DPB supported by the BIOS. The BIOS uses `sdcard` from the cpmtools format definition `/etc/cpmtools/diskdefs`.
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

In this format, one track is reserved for boottrk, and the CP/M system (CCP+BDOS) is stored here on the system disk (`IMAGE00.IMG`).

The SD Card access library in ATmega128 firmware can only write to files that already exist. For this limitation, all data (approximately 8GB) of the disk image file must be generated in advance.

## How to generate a disk image file
`z80/cpm22/image/Makefile` supports the following image generation.

### CP/M2.2 system disk
Generate `DISK00.IMG` based on [CP/M 2.2 BINARY](http://www.cpm.z80.de/download/cpm22-b.zip).
```
make
```
#### When using `z80/cpm22/sys/CPM.SYS`
If you want to use `CPM.SYS` built based on the source code of [CP/M 2.2 ASM SOURCE](http://www.cpm.z80.de/download/cpm2-asm.zip), change `z80/cpm22/image/Makefile` as follows.
```
CCPBDOS   = ../sys/CPM.SYS
#CCPBDOS  = $(CPM22_DIR)/CPM.SYS
```

### Empty disk image
`EMPTY.IMG` will be generated, so copy it to `DISKnn.IMG` and use it, or create a new image based on this.
```
make empty
```

### ZORK I/II/III
```
make zork
```
`ZORK.IMG` will be generated, so copy it to `DISKnn.IMG` and use it.

### Any disk image
You can create any image by using cpmtools' `cpm.cp' to transfer any file onto the disk image.

For example, there are many CP/M binary files at [CP/M software Download site](#cpm-software-download-site). You can also pick up the files and create a disk image that you want.

The following example transfers all files in `./tmp` to an empty disk image and assigns it to the `B:` drive as `DISK01.IMG`.
```
cpmcp -f sdcard EMPTY.IMG ./tmp/*.* 0:
cp EMPTY.IMG DISK01.IMG
```

## CP/M software Download site
- [The Unofficial CP/M Web site](http://www.cpm.z80.de/)
  - [CP/M binary](http://www.cpm.z80.de/binary.html)
  - [CP/M source](http://www.cpm.z80.de/source.html)
- [Commercial CP/M Software](http://www.retroarchive.org/cpm/)
  - [ZORK I/II/III](http://www.retroarchive.org/cpm/games/zork123_80.zip)
