ASXS	:= asxs5p40.zip
ASXSURL	:=　https://shop-pdp.net/_ftp/asxxxx/asxs5p40.zip

TOOLDIR	:= toolchain
TMPDIR	:= $(TOOLDIR)/tmp
AS		:= $(TOOLDIR)/asz80
LD		:= $(TOOLDIR)/aslink
RM		:= rm
CP		:= cp
MKDIR	:= mkdir
MAKE	:= make

%.ihx:%.rel
	$(LD) $<

%.rel:%.asm
	$(AS) -o $(ASFLAGS) $<

$(AS):
	$(MKDIR) -p $(TOOLDIR)
	$(MKDIR) -p $(TMPDIR)
	wget -O $(TMPDIR)/$(ASXS) $(ASXSURL)
	unzip $(TMPDIR)/$(ASXS) -d $(TMPDIR)
	$(MAKE) -C $(TMPDIR)/asxv5pxx/asxmak/linux/build -f makefile asz80
	$(CP) $(TMPDIR)/asxv5pxx/asxmak/linux/exe/asz80 $(TOOLDIR)
	$(MAKE) -C $(TMPDIR)/asxv5pxx/asxmak/linux/build -f makefile aslink
	$(CP) $(TMPDIR)/asxv5pxx/asxmak/linux/exe/aslink  $(TOOLDIR)
	$(RM) -fr $(TMPDIR)
