RM		:= rm
CP		:= cp
MKDIR	:= mkdir
MAKE	:= make

TMPDIR	= ./tmp
DISKDEF	= sdcard
IMAGE   = DISK00.IMG
CCPBDOS = ../62K_CCP_BDOS.BIN
USER	= 0:


$(IMAGE):
	mkfs.cpm -f $(DISKDEF) -b $(CCPBDOS) $(IMAGE)
	dd if=/dev/zero of=$(IMAGE) bs=8192 count=1021 oflag=append conv=notrunc
	cpmcp -f $(DISKDEF) $(IMAGE) $(TMPDIR)/*.* $(USER)
	cpmls -f $(DISKDEF) $(IMAGE)
	echo `wc -c < $(IMAGE)`

ls:
	cpmls -f $(DISKDEF) $(IMAGE)

dump:
	hexdump -C -v $(IMAGE)

clean:
	rm -f $(IMAGE)
