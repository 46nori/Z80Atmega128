AS		:= asz80
LD		:= aslink
CC		:= sdcc

RM		:= rm
CP		:= cp
MKDIR	:= mkdir
MAKE	:= make

echoback.ihx:

%.ihx:%.rel
	$(LD) -i $<

%.rel:%.asm
	$(AS) -l -o $(ASFLAGS) $<

clean:
	rm -f echoback.ihx echoback.rel echoback.lst echoback.hlr
