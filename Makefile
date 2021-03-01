ifdef COMSPEC
DOTEXE:=.exe
else
DOTEXE:=
endif

.PHONY: all
all: nsf2nes$(DOTEXE)


drv.bin: drv.asm
	@64tass -f -o drv.bin drv.asm

nsf2nes$(DOTEXE): nsf2nes.c drv.bin
	@$(CC) $(CFLAGS) -s -Ofast -Wall $< -o $@
