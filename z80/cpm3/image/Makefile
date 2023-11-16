##########################################################
# Create CP/M 3 DISK Image
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
PATCH   := patch

# cpmtools
CPM_MKFS	:= mkfs.cpm
CPM_CP		:= cpmcp
CPM_LS		:= cpmls
CPM_RM		:= cpmrm

# DISK configuration. See /etc/cpmtools/diskdefs
DISKDEF		:= sdcard
USER		:= 0:
TMPDIR		:= ./tmp

##########################################################
# CP/M3 image on CP/M2.2
#    make
#
# Copy IMAGE00.IMG to SD Card.
# Boot CP/M 2.2.
# Enter `SUBMIT GEN` to generate CPM3.SYS and CPMLDR.COM.
# Enter `CPMLDR` to launch CP/M 3. 
# (Note) SUBMIT.COM is for CP/M 2.2; not for CP/M3.
##########################################################
# CCP+BDOS binary for embedding to the DISK image
CPM22_IMAGE	= DISK00.IMG
#CPM2SYS = ../../cpm22/sys/CPM.SYS
CPM2SYS		= $(CPM22_DIR)/CPM.SYS
CPM22_DIR	= $(TMPDIR)/cpm22
CPM3BIN_DIR	= $(TMPDIR)/cpm3bin
CPM3ON2_DIR	= $(TMPDIR)/cpm3on2

# Create DISK image of CP/M 2.2 for SD card using cpmtools
#   CCP+BDOS speficied by $(CPM2SYS) is set to reserved track. 
#   Te following files are copied: 
#      $(CPM22_IMAGE)/SUBMIT.COM
#      $(CPM3BIN_DIR)/*.*
#      $(CPM3SYS_FILES)
$(CPM22_IMAGE):	$(CPM2SYS) $(CPM3ON2_DIR) $(CPM3BIN_DIR)
	$(CPM_MKFS) -f $(DISKDEF) -b $(CPM2SYS) $@
	$(DD) if=/dev/zero of=$@ bs=8192 count=1021 oflag=append conv=notrunc
	$(CPM_CP) -f $(DISKDEF) $@ $(CPM3BIN_DIR)/*.* $(CPM3SYS_FILES) $(USER)
	$(CPM_RM) -f $(DISKDEF) $@ SUBMIT.COM $(USER)
	$(CPM_CP) -f $(DISKDEF) $@ $(CPM22_DIR)/SUBMIT.COM $(USER)
	$(CPM_LS) -f $(DISKDEF) $@
	echo `wc -c < $@`

# Download CP/M 2.2 CPM.SYS and SUBMIT.COM
#   http://www.cpm.z80.de/download/cpm22-b.zip
$(CPM2SYS):
	$(MKDIR) -p $(CPM22_DIR)
	$(WGET) -P $(CPM22_DIR) http://www.cpm.z80.de/download/cpm22-b.zip
	$(UNZIP) -o -d $(CPM22_DIR) $(CPM22_DIR)/cpm22-b.zip CPM.SYS SUBMIT.COM
	$(RM) -f $(CPM22_DIR)/cpm22-b.zip

# Download CP/M 3 files to generate CPM3.SYS and CPMLDR.COM
# Patch LDRBIOS.ASM to fit the 62K CPM.2.2 BIOS.
#   http://www.cpm.z80.de/download/cpm3on2.zip
LDRBIOS			= LDRBIOS.ASM
LDRBIOS_PATCH	= CPM3-LDRBIOS.patch
CPM3SYS_FILES	= $(CPM3ON2_DIR)/$(LDRBIOS) \
				  $(CPM3ON2_DIR)/GBIOS.ASM \
				  $(CPM3ON2_DIR)/SCB.REL \
				  $(CPM3ON2_DIR)/RMAC.COM \
				  $(CPM3ON2_DIR)/LINK.COM \
				  $(CPM3ON2_DIR)/GEN.SUB
#				  $(CPM3ON2_DIR)/BDOS3.SPR \
#				  $(CPM3ON2_DIR)/CPMLDR.REL \
#				  $(CPM3ON2_DIR)/GENCPM.COM \
#				  $(CPM3ON2_DIR)/CCP.COM \

$(CPM3ON2_DIR):
	$(MKDIR) -p $(CPM3ON2_DIR)
	$(WGET) -P $(CPM3ON2_DIR) http://www.cpm.z80.de/download/cpm3on2.zip
	$(UNZIP) -o -d $(CPM3ON2_DIR) $(CPM3ON2_DIR)/cpm3on2.zip $(CPM3_GEN_FILES)
	$(RM) -f $(CPM3ON2_DIR)/cpm3on2.zip
	$(PATCH) --binary -i $(LDRBIOS_PATCH) $(CPM3ON2_DIR)/$(LDRBIOS)

# Download CP/M 3 files
#   http://www.cpm.z80.de/download/cpm3bin.zip
$(CPM3BIN_DIR):
	$(MKDIR) -p $(CPM3BIN_DIR)
	$(WGET) -P $(CPM3BIN_DIR) http://www.cpm.z80.de/download/cpm3bin.zip
	$(UNZIP) -o -d $(CPM3BIN_DIR) $(CPM3BIN_DIR)/cpm3bin.zip
	$(RM) -f $(CPM3BIN_DIR)/cpm3bin.zip

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