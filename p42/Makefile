
#  Plan 42 - Makefile 
#  Based on the makefile for the Lice compiler which was 
#  written by Dale Weiler 


CC      ?= g++
CFLAGS  += -Wall -Wextra -Wno-missing-field-initializers -O3 -std=c99 
LDFLAGS +=

SUBDIRS =            \
      SysCore/Fat12      \
      SysCore/FloppyDisk \
      SysCore/Hal        \
      SysCore/Include    \
      SysCore/Kernel     \
      SysCore/Keyboard   \
      SysCore/Lib        \
      SysCore/proc

VPATH = %.cpp %.c $(SUBDIRS)

P42SOURCES = cpu.cpp dma.cpp gdt.cpp hal.cpp idt.cpp pic.cpp pit.cpp tss.cpp user.cpp fat12.cpp flpydsk.cpp joco.cpp entry.cpp exception.cpp fsys.cpp main.cpp mmngr_phys.cpp mmngr_virtual.cpp panic.cpp sysapi.cpp task.cpp vmmngr_pde.cpp vmmngr_pte.cpp kybrd.cpp cstd.cpp ctype.c stdio.cpp string.cpp 
P42OBJECTS = $(P42SOURCES:.cpp=.o)
P42DEPENDS  = $(P42SOURCES:.cpp=.d)
P42BIN      = p42

all: $(P42BIN) $(TESTBIN)

$(P42BIN): $(P42OBJECTS)
	$(CC) $(LDFLAGS) $(P42OBJECTS) -o $@

c.o:
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(P42OBJECTS)
	rm -f $(TESTOBJECTS)
	rm -f $(P42DEPENDS)
	rm -f $(TESTDEPENDS)
	rm -f $(ARGSDEPENDS)
	rm -f $(P42BIN)
	rm -f $(TESTBIN)
	rm -f $(ARGSBIN)


