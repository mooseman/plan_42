# Makefile for Turbo/Borland C

# defines
MAKEDEP	=makefile
TCDIR	=c:\t
LIBDIR	=$(TCDIR)\lib
BINDIR	=$(TCDIR)\bin
AS	=nasm -g -f obj -dTINY=1
CC	=$(BINDIR)\tcc -v -mt -w -O2 -d -Z
LD	=$(BINDIR)\tlink /3 /v /x /c
OBJS	=mbload.obj lib.obj

# targets
all : mbload.exe
#all : mbload.com

clean :
	deltree /y *.exe *.com *.obj *.err

# implicit rules
.c.obj :
	$(CC) -c -o$*.obj $<

.asm.obj :
	$(AS) -o$*.obj $<

# dependencies
mbload.obj :	$(MAKEDEP) mbload.c

lib.obj :	$(MAKEDEP) lib.asm

# explicit rules
mbload.exe : $(OBJS) $(MAKEDEP)
	$(LD) $(LIBDIR)\c0t.obj $(OBJS),mbload.exe,,$(LIBDIR)\cs.lib

#mbload.com : $(OBJS) $(MAKEDEP)
#	$(LD) /t $(LIBDIR)\c0t.obj $(OBJS),mbload.com,,$(LIBDIR)\cs.lib
