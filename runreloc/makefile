.SUFFIXES: .exe

# defines
# MAKEDEP	=makefile
CC	=gcc -g -O2 -Wall -W
LD	=gcc -g
RUN	=run.o coff.o djcoff.o elf.o console.o
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

# dependencies
tetris.o:	tetris.c	$(MAKEDEP)

libc.o: 	libc.c		$(MAKEDEP)

# explicit rules
run.exe: $(RUN) $(MAKEDEP)
	$(LD) -o$@ $(RUN)

tetris.r: $(TETRIS) $(MAKEDEP)
# -d = force common symbols to be defined
	ld -d -r -nostdlib -o$@ $(TETRIS)
# perverse linker script; works only with DJGPP based on GCC 2.x
#	ld -d -r -Tperverse.ld -nostdlib -o$@ $(TETRIS)
