Runreloc - loads relocatable (.o) file into memory and runs it.
Chris Giese <geezer@execpc.com>, http://my.execpc.com/~geezer
Rewritten for clarity April 3, 2003
This code is public domain (no copyright).
You can do whatever you want with it.

This code supports DJGPP COFF, Win32 PE COFF (not the same!),
and Linux ELF. It should be compiled with GCC.

Because it's difficult to distinguish between DJGPP COFF and
Win32 PE COFF relocatable files, only one of these two formats
is supported at a time:

Makefile    System                  Supported .o formats
---------   ----------------------- ---------------------
makefile    GCC for DOS (DJGPP)     ELF and DJGPP COFF
linux.mak   GCC for Linux           ELF and DJGPP COFF
ming.mak    GCC for Win32 (MinGW32) ELF and Win32 PE COFF

After building, type
	run tetris.r

================================================================
GCC 'common' variables are not supported
================================================================
"Common" variables are non-static, non-initialized, non-extern
globals. GNU C supports these by default; other compilers
enable common variables only if you use the proper command-line
options.

This code does not handle common variables in the .o file (you
get a message about "undefined external symbol"). If you can
make all globals static, you should do so. Otherwise, use ld to
allocate memory for such variables like this:

	ld -d -r -o app.r app.o

Then run app.r instead of app.o

ld option -d is equivalent to the FORCE_COMMON_ALLOCATION
command in a linker script.

================================================================
The entire relocatable file is loaded into memory
================================================================
Loading the entire .o file into memory, rather than loading
individual sections, makes relocation more complex if file
alignment != memory alignment, as is usually the case. Even if
the two alignments are equal, relocations will be wrong for
sections that come after section .bss, because .bss occupies no
space in the file:

    objdump -h test.r

    test.r:     file format coff-go32
    Sections:
    Idx Name  Size      VMA       LMA       File off  Algn
      0 .text 000000f0  00000000  00000000  000000b4  2**4
              CONTENTS, ALLOC, LOAD, RELOC, CODE
      1 .data 00000020  000000f0  000000f0  000001a4  2**4
              CONTENTS, ALLOC, LOAD, DATA
      2 .bss  00000004  00000110  00000110  00000000  2**2
              ALLOC
      3 .dll  00000084  00000114  00000114  000001c4  2**2
              CONTENTS, ALLOC, LOAD, RELOC, CODE

The VMA of section .dll (address relative to .text) is 0x114.
This is where the code/data in .dll "wants" to be. But the load
address of section .dll (also relative to .text) is
(0x1C4 - 0xB4) == 0x110. This is where the code/data in .dll
actually is. The difference (4 bytes) is the size of the BSS.

Nevertheless, this code does load the entire file into memory.
Everything is properly relocated to run without error (or it
should be, anyway).

================================================================
Other notes
================================================================
Arrow keys _still_ don't work under Linux when playing Tetris

The 'perverse.ld' linker script works with Linux GCC 3.x
and with DJGPP based on GCC 2.x. It does not work with DJGPP
based on GCC 3.x, nor with any version of MinGW available to me.

Tetris built with these ELF binutils for DJGPP:
	http://www.multimania.com/placr/binutils.html
now works properly. Directions:

1. In file ELF.C, in function get_elf_sym(), find this line
	err = lookup_external_symbol(sym_name, sym_val, 0);
   and change it to
	err = lookup_external_symbol(sym_name, sym_val, 1);
   to enable leading underscores.
2. Use as-elf.exe as the assembler when compiling tetris.c and
   libc.c. Compile without debug info, or as-elf.exe will choke.
3. Use ld-elf.exe to link tetris.o and libc.o into tetris.r

Using objcopy-elf.exe to convert tetris.r from COFF to ELF
will not work (the relocations are not properly converted).
