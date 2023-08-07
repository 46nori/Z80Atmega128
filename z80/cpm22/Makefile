AS		:= asz80
LD		:= aslink

RM		:= rm
CP		:= cp
MKDIR	:= mkdir
MAKE	:= make
OC      := objcopy

TMPDIR	= ./tmp


cpm.ihx: $(TMPDIR)/cpm22.rel ipl.rel bios.rel
	$(LD) -i $@ ipl.rel $(TMPDIR)/cpm22.rel bios.rel

%.rel:%.asm
	$(AS) -l -o $(ASFLAGS) $<

#
# Build CP/M 2.2 from source code
#
62K_CCP_BDOS.BIN: $(TMPDIR)/cpm22.ihx
	$(OC) -I ihex -O binary $< $@

$(TMPDIR)/cpm22.ihx: $(TMPDIR)/cpm22.rel
	$(LD) -i $@ $(TMPDIR)/cpm22.rel

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
	rm -f ipl.rel ipl.lst ipl.ihx ipl.hlr
	rm -f bios.rel bios.lst bios.ihx bios.hlr
	rm -f cpm22.rel cpm22.ihx
	rm -f CCP_BDOS.BIN