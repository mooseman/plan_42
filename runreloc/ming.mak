.SUFFIXES: .exe

# defines
# MAKEDEP	=makefile
CC	=gcc -g -O2 -Wall -W
LD	=gcc -g
RUN	=run.o coff.o pecoff.o elf.o console.o windows.o
TETRIS	=tetris.o libc.o

# targets
all: run.exe tetris.r

clean:
	deltree /y run.exe *.o tetris.r

# implicit rules
.c.o:
	$(CC) -c -o$@ $<

# dependencies
run.o:		run.c		$(MAKEDEP) defs.h

console.o:	console.c	$(MAKEDEP)

coff.o:		coff.c		$(MAKEDEP) defs.h

djcoff.o:	djcoff.c	$(MAKEDEP) defs.h

elf.o:		elf.c		$(MAKEDEP) defs.h

pecoff.o:	pecoff.c	$(MAKEDEP) defs.h

windows.o:	windows.c	$(MAKEDEP)

# dependencies
tetris.o:	tetris.c	$(MAKEDEP)

libc.o: 	libc.c		$(MAKEDEP)

# explicit rules
run.exe: $(RUN) $(MAKEDEP)
	$(LD) -o$@ $(RUN)

tetris.r: $(TETRIS) $(MAKEDEP)
# -d = force common symbols to be defined
	ld -d -r -nostdlib -o$@ $(TETRIS)
# perverse linker script causes MinGW ld to crash
#	ld -d -r -Tperverse.ld -nostdlib -o$@ $(TETRIS)
