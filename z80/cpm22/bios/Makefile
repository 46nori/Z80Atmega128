#
# Create Intel hex files of CP/M BIOS
#
# ASxxxx Cross Assembler is required.
#
RM		:= rm
MAKE	:= make

# ASxxxx Cross Assembler
AS		:= asz80
LD		:= aslink
ASFLAGS := 

TMPDIR	= ./tmp

bios.ihx: bios.rel
	$(LD) -i $@ $<

bios.rel: bios.asm
	$(AS) -l -o $(ASFLAGS) $<

.PHONY: clean
clean:
	@$(RM) -f *.ihx *.rel *.lst *.hlr
	