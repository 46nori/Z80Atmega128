AS		:= asz80
LD		:= aslink

RM		:= rm
CP		:= cp
MKDIR	:= mkdir
MAKE	:= make

TMPDIR	= ./tmp


cpm22.ihx: $(TMPDIR)/cpm22.rel ipl.rel bios.rel
	$(LD) -i $@ ipl.rel $(TMPDIR)/cpm22.rel bios.rel

%.rel:%.asm
	$(AS) -l -o $(ASFLAGS) $<

#
# Build CP/M 2.2 from source code
#
$(TMPDIR)/cpm22.rel: $(TMPDIR)/cpm22.asm
	$(AS) -l -o $(ASFLAGS) $<

$(TMPDIR)/cpm22.asm: $(TMPDIR)/CPM22.Z80
	patch -i CPM22-asz80.patch -o $@ $<

$(TMPDIR)/CPM22.Z80: $(TMPDIR)/cpm2-asm.zip
	unzip -o -d $(TMPDIR) $(TMPDIR)/cpm2-asm.zip CPM22.Z80
	touch $@

$(TMPDIR)/cpm2-asm.zip:
	$(MKDIR) -p $(TMPDIR)
	wget -P $(TMPDIR) http://www.cpm.z80.de/download/cpm2-asm.zip

clean:
	rm -fr $(TMPDIR)
