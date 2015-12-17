

#  Demo1 - Makefile -  Stage 2

#  Author -  mooseman  

all:

	nasm -f bin stage2.asm -o krnldr.sys

	cp ~/plan_42/Demo1/Stage1/floppy.img .

	sudo mount -o loop floppy.img /media/floppy/

	#cp krnldr.sys floppy.img
	dd if=krnldr.sys of=floppy.img conv=notrunc bs=512 seek=2 skip=2 

	sudo umount /media/floppy/









