Nanos ver 0.004	Created by Peter Hultqvist, peter.hultqvist@neocoder.net
Release date 2003-12-02
http://www.neocoder.net/nanos/

This is a working Loader + Nanos Kernel which will load the files
specified in nanos.dat into memory.
There is however no UI at the moment

Please send comments and questions to: nanos@neocoder.net

Help is needed:
If you want to help witing drivers or applications for nanos you are welcome.
Just contact me and I will help you.

	Peter Hultqvist, peter.hultqvist@neocoder.net


=====================================================================

Contents:
1 Files used in Nanos
2 Usage
3 Kernel Status

=====================================================================
1 Files used in Nanos:

Boot.bin
	The boot sector of the disk.
	Loads nanos.bin and nanos.dat.
	It also loads all other files listed in nanos.dat
	Initiates Flat Pmode and jump to code from nanos.bin
	
Nanos.bin
	Code to initiate alla memory structures in nanos.
	The contents of nanos.bin and all its listed files are
	moved into a module where the first file recieves control.
	Start Nanos.
	
Nanos.dat
	List of all files to be loaded.
	The first entry will recieve control from the loader and must
	therefore be binary.
	An entry is 16 byte and contains the following:
	byte 0	':' character
	byte 1	'1'-'3' CPL of loaded file(CPL 0 is only for the kernel)
	byte 2	Unknown - Any ideas
	byte 3-13 Filename(in FAT format) of file to be loaded.
		Example: "driver.elf" -(FAT)-> "DRIVER  ELF"
	byte 14-15 cr+lf
		Makes it easy to edit the file in a text editor
		These two bytes is also used to specify the location of
		the loaded file

elf_load.bin
	First entry in Nanos.dat
	Create a Module(and a running process) for each file in Nanos.dat
	All files must be in Executable-ELF format

test.elf
	A test program to show how module can be loaded
	
=====================================================================
2 Usage

Just boot from the disk and enjoy.

boot.bin will print "Loading:" in white text.
	After that you will see a '.' for each sector read and a '/'
	for each new file loaded(including nanos.bin and nanos.dat)
	If an error occurs a 'X' will be written in the sequence and
	the loader will stop.

The Loader(nanos.bin) will write some debug data on the screen in
	yellow text. All debug data is also sent to port com1 using
	9600bps, 1 stop, np.
	
Test.elf will also send bytes on com1. 
	Since there will be three test.elf running you will see tree
	different charachters written on the port.
	You can make more copies of the ":1-TEST    ELF" line in
	nanos.dat to have more processess running.

Thats all
	At the moment there are no Video drivers or command shell to explore.
	But that will com in the future.

=====================================================================
3 Kernel Status

	Mostly all kernel functions are working fine, but there are
	probaly lots of bugs. I have almost found at least one bug in
	each function used by elf_load.bin.
	
	When Nanos is loaded the maximum size of loaded files must not
	exceed approx. 500kB. That's because all files are loaded
	in real mode. At the moment isn't the loader checking this, so
	if the screen gets over-written by some file its probaly because
	boot.bin has come up to address 0C0000h when loading all the files.
	
	nanos.dat must me exactly as specified. If not the loader might
	skip a file or stop loading
	
	I'm not sure but i think nanos.dat must not be larger than one sector.
	
	
	

