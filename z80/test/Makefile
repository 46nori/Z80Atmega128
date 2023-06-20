TOOLDIR	:= ../toolchain
AS		:= $(TOOLDIR)/asz80
LD		:= $(TOOLDIR)/aslink

RM		:= rm
CP		:= cp
MKDIR	:= mkdir
MAKE	:= make

echoback.ihx:

%.ihx:%.rel
	$(LD) -i $<

%.rel:%.asm
	$(AS) -o $(ASFLAGS) $<

toolchain:
	$(MAKE) -C $(TOOLDIR)