#
# Create CP/M DISK Image
#
# cpmtool is required.
#
RM		:= rm
CP		:= cp
MKDIR	:= mkdir
MAKE	:= make

WGET	:= wget
UNZIP	:= unzip
DD		:= dd
HEXDUMP	:= hexdump

# cpmtools
CPM_MKFS	:= mkfs.cpm
CPM_CP		:= cpmcp
CPM_LS		:= cpmls

# DISK configuration. See /etc/cpmtools/diskdefs
DISKDEF	= sdcard

# CCP+BDOS binary for embedding to the DISK image
#CCPBDOS = ../sys/CPM.SYS
CCPBDOS = $(TMPDIR)/CPM.SYS

USER	= 0:
IMAGE   = DISK00.IMG
TMPDIR	= ./tmp

# Create DISK image of CP/M 2.2 for SD card using cpmtools
#   CCP+BDOS speficied by $(CCPBDOS) is set to reserved track. 
#   Files are copy from $(TMPDIR).
$(IMAGE):	$(TMPDIR)
	$(CPM_MKFS) -f $(DISKDEF) -b $(CCPBDOS) $(IMAGE)
	$(DD) if=/dev/zero of=$(IMAGE) bs=8192 count=1021 oflag=append conv=notrunc
	$(CPM_CP) -f $(DISKDEF) $(IMAGE) $(TMPDIR)/*.* $(USER)
	$(CPM_LS) -f $(DISKDEF) $(IMAGE)
	echo `wc -c < $(IMAGE)`

# Download CP/M 2.2 binay:
#   http://www.cpm.z80.de/download/cpm22-b.zip
$(TMPDIR):
	$(MKDIR) -p $(TMPDIR)
	$(WGET) -P $(TMPDIR) http://www.cpm.z80.de/download/cpm22-b.zip
	$(UNZIP) -o -d $(TMPDIR) $(TMPDIR)/cpm22-b.zip
	$(RM) -f $(TMPDIR)/cpm22-b.zip

# list files in the image file
.PHONY: ls
ls:
	$(CPM_LS) -f $(DISKDEF) $(IMAGE)

# Hex dump the image file
.PHONY: dump
dump:
	$(HEXDUMP) -C -v $(IMAGE)

# clean up
.PHONY: clean
clean:
	@$(RM) -f $(IMAGE)
	@$(RM) -fr $(TMPDIR)