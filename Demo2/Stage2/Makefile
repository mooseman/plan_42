

#  Demo1 - Makefile -  Stage 2

#  Author -  mooseman  

all:

	nasm -f bin stage2.asm -o krnldr.sys

	cp ~/plan_42/Demo2/Stage1/floppy.img .
	
	dd if=krnldr.sys of=floppy.img conv=notrunc iflag=count_bytes \
	oflag=append,seek_bytes seek=451 count=65











