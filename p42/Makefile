

#  Plan 42 makefile 

CC=g++

CFLAGS=-c -Wall

LDFLAGS=

VPATH *.cpp SysCore SysCore/Fat12 SysCore/FloppyDisk SysCore/Hal SysCore/Include
			SysCore/Kernel SysCore/Keyboard SysCore/Lib SysCore/proc



SOURCES=*.cpp 

OBJECTS=$(SOURCES:.cpp=.o)

# Use nasm to generate its Makefile dependencies
# nasm -M Stage2.asm > Stage2.dep 

EXECUTABLE=plan42

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

#  TO DO - need to do the SysBoot directory and 
#  set up rules for nasm here 


