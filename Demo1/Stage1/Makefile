

#  Demo1 - Makefile -  Stage 1

#  Author -  mooseman  

all:
	nasm -f bin boot1.asm -o boot1.bin

	# Create the floppy image
	dd if=/dev/zero of=floppy.img bs=512 count=2880

	# Create the loopback device
	sudo losetup /dev/loop0 floppy.img

	# Format the floppy image
	sudo mkdosfs -F 12 /dev/loop0

	# Mount the floppy image
	sudo mount /dev/loop0 /mnt -t msdos -o "fat=12"

	# Copy the boot1.bin file onto the floppy image. 
	# The problem must be here as the other commands are as 
	# per the instructions (apart from the sudo).   	
	#dd if=boot1.bin of=floppy.img conv=notrunc 	
	sudo dd if=boot1.bin of=/dev/loop0 conv=notrunc 

	# Unmount and remove the loopback device. 
	sudo umount /mnt
	sudo losetup -d /dev/loop0

	
		
	
	











