

#  Demo1 - Makefile -  Stage 1

#  Author -  mooseman  

all:
	nasm -f bin boot1.asm -o boot1.bin

	dd bs=512 count=2880 if=/dev/zero of=floppy.img

	mkfs.msdos floppy.img
	
	dd if=boot1.bin of=floppy.img bs=3 count=1 conv=notrunc 
	dd if=boot1.bin of=floppy.img conv=notrunc iflag=skip_bytes,count_bytes \
	oflag=append skip=62 count=388










