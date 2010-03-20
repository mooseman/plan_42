.SUFFIXES: .exe

# defines
# MAKEDEP	=makefile
CC	=gcc -g -O2 -Wall -W
LD	=gcc -g
RUN	=run.o coff.o djcoff.o elf.o linux.o
TETRIS	=tetris.o libc.o

# targets
all: run tetris.r

clean:
	rm -f run *.o tetris.r

# implicit rules
.c.o:
	$(CC) -c -o$@ $<

# dependencies
run.o:		run.c		$(MAKEDEP) defs.h

coff.o:		coff.c		$(MAKEDEP) defs.h

djcoff.o:	djcoff.c	$(MAKEDEP) defs.h

elf.o:		elf.c		$(MAKEDEP) defs.h

pecoff.o:	pecoff.c	$(MAKEDEP) defs.h

linux.o:	linux.c		$(MAKEDEP)

# dependencies
tetris.o:	tetris.c	$(MAKEDEP)

libc.o: 	libc.c		$(MAKEDEP)

# explicit rules
run: $(RUN) $(MAKEDEP)
	$(LD) -o$@ $(RUN)

tetris.r: $(TETRIS) $(MAKEDEP)
# -d = force common symbols to be defined
#	ld -d -r -nostdlib -o$@ $(TETRIS)
# perverse linker script; works with GCC 3.x ...
	ld -d -r -Tperverse.ld -nostdlib -o$@ $(TETRIS)
