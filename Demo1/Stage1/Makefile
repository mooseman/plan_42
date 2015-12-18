

#  Demo1 - Makefile -  Stage 1

#  Author -  mooseman  

all:
	nasm -f bin boot1.asm -o boot1.bin

	dd bs=512 count=2880 if=/dev/zero of=floppy.img

	mkfs.msdos floppy.img

	sudo mount -o loop floppy.img /media/floppy/

	#cp boot1.bin floppy.img
	dd if=boot1.bin of=floppy.img bs=3 count=1 conv=notrunc 
	dd if=boot1.bin of=floppy.img conv=notrunc ibs=62 skip=1 obs=388 count=1

	sudo umount /media/floppy/









