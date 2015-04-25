
#  Plan 42 - Makefile 



CC      = g++
LD 	= g++
NASM	= nasm 
 

CFLAGS  = -Wall -Wextra -Wno-missing-field-initializers -m32 -O3 -std=c99 
LDFLAGS = -m32
NASMFLAGS = -f elf -F stabs 


PROG = p42

OBJS = Boot1.o Stage2.o $(PROG).o cpu.o dma.o gdt.o hal.o idt.o 


#default: $(PROG)
all:	$(PROG)

$(PROG): $(OBJS)
	$(LD) $(LFLAGS) $(OBJS)  -o $(PROG)

Boot1.o: Boot1.asm
	$(NASM) $(NASMFLAGS) Boot1.asm

Stage2.o: Stage2.asm
	$(NASM) $(NASMFLAGS) Stage2.asm


cpu.o:	cpu.cpp
	$(CC) $(CFLAGS) cpu.cpp

dma.o:	dma.cpp
	$(CC) $(CFLAGS) dma.cpp		

gdt.o:	gdt.cpp
	$(CC) $(CFLAGS) gdt.cpp	

hal.o:	hal.cpp
	$(CC) $(CFLAGS) hal.cpp

idt.o:	idt.cpp
	$(CC) $(CFLAGS) idt.cpp



SUBDIRS = SysBoot/Stage1	\
        SysBoot/Stage2		\
        SysCore/Fat12		\
        SysCore/FloppyDisk	\
        SysCore/Hal		\
        SysCore/Include		\
        SysCore/Kernel		\
        SysCore/Keyboard	\
        SysCore/Lib		\
        SysCore/proc

VPATH = %.cpp %.c %.asm $(SUBDIRS)



P42SOURCES = cpu.cpp dma.cpp gdt.cpp hal.cpp idt.cpp pic.cpp pit.cpp tss.cpp user.cpp fat12.cpp flpydsk.cpp joco.cpp entry.cpp exception.cpp fsys.cpp main.cpp mmngr_phys.cpp mmngr_virtual.cpp panic.cpp sysapi.cpp task.cpp vmmngr_pde.cpp vmmngr_pte.cpp kybrd.cpp cstd.cpp ctype.c stdio.cpp string.cpp 





