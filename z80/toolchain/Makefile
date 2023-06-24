# ASxxxx Cross Assemblers
ASXURL	:= https://shop-pdp.net/_ftp/asxxxx/asxs5p40.zip
ASXZIP	:= asxs5p40.zip
ASXTXT	:= asxs5p40.txt
ASXTMP	:= ./asxv5pxx
AS		:= asz80
LD		:= aslink
ASXDIR	:= /usr/local/bin

# SDCC Small Device C Compiler
#SDCURL	:= http://sourceforge.net/projects/sdcc/files/snapshot_builds/sdcc-src/sdcc-src-20230622-14110.tar.bz2
#SDCBZ2	:= sdcc-src-20230622-14110.tar.bz2
#SDCTMP	:= sdcc
#CC		:= sdcc
#SDCDIR	:= /usr/local

# Destination directory
TOOLDIR	:= .

RM		:= rm
CP		:= cp
LN      := ln
MKDIR	:= mkdir
MAKE	:= make

all:	$(ASXDIR)/$(AS) $(ASXDIR)/$(LD) #$(SDCDIR)/bin/$(CC)

clean:
	$(MAKE) clean_asx
#	$(MAKE) clean_sdcc

# ASxxxx build rules
$(ASXDIR)/$(AS):	$(ASXTMP)
	$(MAKE) -C $(ASXTMP)/asxmak/linux/build -f makefile $(AS)
	sudo $(CP) $(ASXTMP)/asxmak/linux/exe/$(AS) $(ASXDIR)/$(AS)

$(ASXDIR)/$(LD):	$(ASXTMP)
	$(MAKE) -C $(ASXTMP)/asxmak/linux/build -f makefile $(LD)
	sudo $(CP) $(ASXTMP)/asxmak/linux/exe/$(LD) $(ASXDIR)/$(LD)

$(ASXTMP):
	wget -O $(ASXZIP) $(ASXURL)
	unzip -L -a $(ASXZIP)
	$(RM) -f $(ASXZIP)

clean_asx:
	sudo $(RM) -f $(ASXDIR)/$(AS) $(ASXDIR)/$(LD)
	$(RM) -fr $(ASXZIP) $(ASXTXT) $(ASXTMP)

# SDCC build rules
#$(SDCDIR)/bin/$(CC):
#	wget -O $(SDCBZ2) $(SDCURL)
#	tar xjf $(SDCBZ2)
#	$(RM) -f $(SDCBZ2)
#	sudo $(CP) -r $(SDCTMP)/bin/* $(SDCDIR)/bin
#	sudo $(CP) -r $(SDCTMP)/share/* $(SDCDIR)/share
#
#clean_sdc:
#	sudo $(RM) -f $(DISTDIR)/bin/$(CC)
#	sudo $(RM) -fr $(SDCDIR)/share/sdcc
#	$(RM) -fr $(SDCBZ2) $(SDCTMP)