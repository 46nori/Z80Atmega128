RM		:= rm
CP		:= cp
MKDIR	:= mkdir
MAKE	:= make

TMPDIR	= ./tmp
DISKDEF	= sdcard
IMAGE   = DISK00.IMG
#CCPBDOS = ../62K_CCP_BDOS.BIN
CCPBDOS = $(TMPDIR)/CPM.SYS
USER	= 0:


$(IMAGE):	$(TMPDIR)
	mkfs.cpm -f $(DISKDEF) -b $(CCPBDOS) $(IMAGE)
	dd if=/dev/zero of=$(IMAGE) bs=8192 count=1021 oflag=append conv=notrunc
	cpmcp -f $(DISKDEF) $(IMAGE) $(TMPDIR)/*.* $(USER)
	cpmls -f $(DISKDEF) $(IMAGE)
	echo `wc -c < $(IMAGE)`

$(TMPDIR):
	$(MKDIR) -p $(TMPDIR)
	wget -P $(TMPDIR) http://www.cpm.z80.de/download/cpm22-b.zip
	unzip -o -d $(TMPDIR) $(TMPDIR)/cpm22-b.zip
	$(RM) $(TMPDIR)/cpm22-b.zip

ls:
	cpmls -f $(DISKDEF) $(IMAGE)

dump:
	hexdump -C -v $(IMAGE)

clean:
	@$(RM) -f $(IMAGE)
	@$(RM) -fr $(TMPDIR)