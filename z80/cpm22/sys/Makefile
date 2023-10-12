#
# Create Intel hex files of CP/M and BIOS, and CPM.SYS (CCP+BDOS)
#
# ASxxxx Cross Assembler is required.
#
RM		:= rm
CP		:= cp
MKDIR	:= mkdir
MAKE	:= make
OC      := objcopy
WGET	:= wget
UNZIP	:= unzip

# ASxxxx Cross Assembler
AS		:= asz80
LD		:= aslink

TMPDIR	= ./tmp


BIOS	= ../bios

.PHONY: all
all:	cpm.ihx CPM.SYS

#
# IPL + CCP/BDOS + BIOS (INTEL HEX)
#
cpm.ihx: $(TMPDIR)/cpm22.rel ipl.rel $(BIOS)/bios.rel
	$(LD) -i $@ ipl.rel $(TMPDIR)/cpm22.rel $(BIOS)/bios.rel

ipl.rel: ipl.asm
	$(AS) -o $(ASFLAGS) $<

#
# BIOS 
#
$(BIOS)/bios.rel: 
	$(MAKE) -C $(BIOS)

#
# Build CP/M 2.2(CCP+BDOS) binay from source code
#
CPM.SYS: $(TMPDIR)/cpm22.ihx
	$(OC) -I ihex -O binary $< $@

$(TMPDIR)/cpm22.ihx: $(TMPDIR)/cpm22.rel
	$(LD) -i $@ $<

$(TMPDIR)/cpm22.rel: $(TMPDIR)/cpm22.asm
	$(AS) -l -o $(ASFLAGS) $<

# Patch CPM22.Z80 to assemble by asz80
$(TMPDIR)/cpm22.asm: $(TMPDIR)/CPM22.Z80
	patch -i CPM22-asz80.patch -o $@ $<

# Extract CP/M 2.2 source code written in Z80 mnemonic
$(TMPDIR)/CPM22.Z80: $(TMPDIR)/cpm2-asm.zip
	$(UNZIP) -o -d $(TMPDIR) $(TMPDIR)/cpm2-asm.zip CPM22.Z80
	touch $@

# Download CP/M 2.2 source archive
$(TMPDIR)/cpm2-asm.zip:
	$(MKDIR) -p $(TMPDIR)
	$(WGET) -P $(TMPDIR) http://www.cpm.z80.de/download/cpm2-asm.zip


.PHONY: clean
clean:
	@$(RM) -fr $(TMPDIR)
	@$(RM) -f CPM.SYS
	@$(RM) -f *.ihx *.rel *.lst *.hlr
	