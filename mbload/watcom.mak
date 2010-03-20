# Makefile for Watcom C

# defines
#MAKEDEP	=watcom.mak
AS	=nasm -g -f obj -dTINY=1 -d__WATCOMC__=1
CC	=wcc -zq -0 -d2 -hw -ox -w=9 -zc -zp1 -ms -fr=nul
LD	=wlink OP q D w a SYS com
OBJS	=PFXmbload.obj PFXlib.obj

# targets
all : mbload.exe
#all : mbload.com

clean :
	deltree /y *.exe *.com *.obj *.err

# implicit rules
.c.obj :
	$(CC) -fo=$@ $[.

.asm.obj :
	$(AS) -o$@ $[.

# dependencies
mbload.obj :	$(MAKEDEP) mbload.c

lib.obj :	$(MAKEDEP) lib.asm

# explicit rules
mbload.exe : $(OBJS:PFX=) $(MAKEDEP)
	$(LD) N $@ $(OBJS:PFX=F )
