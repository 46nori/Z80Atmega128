TOOLDIR	:= .
ASXZIP	:= asxs5p40.zip
ASXURL	:= https://shop-pdp.net/_ftp/asxxxx/asxs5p40.zip
ASXDIR	:= ./asxv5pxx
AS		:= asz80
LD		:= aslink

RM		:= rm
CP		:= cp
LN      := ln
MKDIR	:= mkdir
MAKE	:= make

all:	$(TOOLDIR)/$(AS) $(TOOLDIR)/$(LD)

$(TOOLDIR)/$(AS):	$(ASXDIR)
	$(MAKE) -C $(ASXDIR)/asxmak/linux/build -f makefile $(AS)
	$(LN) -s $(ASXDIR)/asxmak/linux/exe/$(AS) $(TOOLDIR)/$(AS)

$(TOOLDIR)/$(LD):	$(ASXDIR)
	$(MAKE) -C $(ASXDIR)/asxmak/linux/build -f makefile $(LD)
	$(LN) -s $(ASXDIR)/asxmak/linux/exe/$(LD) $(TOOLDIR)/$(LD)

$(ASXDIR):
	wget -O $(ASXZIP) $(ASXURL)
	unzip $(ASXZIP)
	$(RM) $(ASXZIP)

clean:
	$(RM) $(TOOLDIR)/$(AS) $(TOOLDIR)/$(LD)
	$(RM) -fr $(ASXDIR)
