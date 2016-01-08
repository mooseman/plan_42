

#  Demo1 - Makefile -  Stage 2

#  Author -  mooseman  

all:

	nasm -f bin stage2.asm -o KRNLDR.SYS

	cp ~/plan_42/Demo1/Stage1/floppy.img .
	
	dd if=KRNLDR.SYS of=floppy.img conv=notrunc 
	
	














