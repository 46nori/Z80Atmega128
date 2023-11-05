##########################################################
# Create CP/M DISK Image
#
# cpmtool is required.
##########################################################
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
DISKDEF		:= sdcard
USER		:= 0:
TMPDIR		:= ./tmp

##########################################################
# CP/M2.2 image
#    make
#
# Copy IMAGE00.IMG to SD Card.
##########################################################
# CCP+BDOS binary for embedding to the DISK image
#CCPBDOS = ../sys/CPM.SYS
CCPBDOS		= $(CPM22_DIR)/CPM.SYS
CPM22_IMAGE	= DISK00.IMG
CPM22_DIR	= $(TMPDIR)/cpm22

# Create DISK image of CP/M 2.2 for SD card using cpmtools
#   CCP+BDOS speficied by $(CCPBDOS) is set to reserved track. 
#   Files are copy from $(CPM22_IMAGE).
$(CPM22_IMAGE):	$(CPM22_DIR)
	$(CPM_MKFS) -f $(DISKDEF) -b $(CCPBDOS) $@
	$(DD) if=/dev/zero of=$@ bs=8192 count=1021 oflag=append conv=notrunc
	$(CPM_CP) -f $(DISKDEF) $@ $(CPM22_DIR)/*.* $(USER)
	$(CPM_LS) -f $(DISKDEF) $@
	echo `wc -c < $@`

# Download CP/M 2.2 binay:
#   http://www.cpm.z80.de/download/cpm22-b.zip
$(CPM22_DIR):
	$(MKDIR) -p $(CPM22_DIR)
	$(WGET) -P $(CPM22_DIR) http://www.cpm.z80.de/download/cpm22-b.zip
	$(UNZIP) -o -d $(CPM22_DIR) $(CPM22_DIR)/cpm22-b.zip
	$(RM) -f $(CPM22_DIR)/cpm22-b.zip

##########################################################
# Empty disk image
#    make empty
#
# Rename EMPTY.IMG with IMAGEnn.IMG and copy it to SD Card.
# (nn is 00 to 04)
##########################################################
EMPTY_IMAGE	= EMPTY.IMG

.PHONY: empty
empty:	$(EMPTY_IMAGE)

$(EMPTY_IMAGE):
	$(CPM_MKFS) -f $(DISKDEF) $@
	$(DD) if=/dev/zero of=$@ bs=8192 count=1021 oflag=append conv=notrunc
	$(CPM_LS) -f $(DISKDEF) $@
	echo `wc -c < $@`

##########################################################
# ZORK I/II/III image
#    make zork
#
# Rename ZORK.IMG with IMAGEnn.IMG and copy it to SD Card.
# (nn is 00 to 04)
##########################################################
ZORK_IMAGE	= ZORK.IMG
ZORK_DIR = $(TMPDIR)/zork

.PHONY: zork
zork:	$(ZORK_IMAGE)

# Create DISK image of ZORK I/II/III
$(ZORK_IMAGE):	$(ZORK_DIR)
	$(CPM_MKFS) -f $(DISKDEF) $@
	$(DD) if=/dev/zero of=$@ bs=8192 count=1021 oflag=append conv=notrunc
	$(CPM_CP) -f $(DISKDEF) $@ $(ZORK_DIR)/*.* $(USER)
	$(CPM_LS) -f $(DISKDEF) $@
	echo `wc -c < $@`

# Download ZORK I/II/III
$(ZORK_DIR):
	$(MKDIR) -p $(ZORK_DIR)
	$(WGET) -P $(ZORK_DIR) http://www.retroarchive.org/cpm/games/zork123_80.zip
	$(UNZIP) -o -d $(ZORK_DIR) $(ZORK_DIR)/zork123_80.zip
	$(RM) -f $(ZORK_DIR)/zork123_80.zip

##########################################################
# Utilities
#    make ls|dump IMAGE=<image file>
#    make clean
##########################################################
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
	@$(RM) -f *.IMG *.img
	@$(RM) -fr $(TMPDIR)