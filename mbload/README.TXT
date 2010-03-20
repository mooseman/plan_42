MBLOAD-C: a Multiboot-compatible kernel loader
that runs from the DOS command prompt.

Chris Giese <geezer@execpc.com>, http://my.execpc.com/~geezer
This code is public domain (no copyright).
You can do whatever you want with it.

Version 0.51, released Nov 24, 2004
- Changed "mov (seg_reg),ecx" to "mov (seg_reg),cx" in LIB.ASM
  The "one byte smaller" comment applied only to NASM 0.97,
  which erroneously added an operand size override prefix
  if you MOVed a 16-bit register into a segment register in
  a BITS 32 code segment.

Version 0.5, released Dec 3, 2003
- It now works with FreeDOS
- Fixed entry point bugs that caused loader to fail with
  Multiboot-ELF kernels but work properly with Multiboot-kludge
  kernels (or vice-versa).

Version 0.45, released Aug 21, 2003
- Fixed mboot_range_t so BIOS memory map is displayed properly
- In LIB.ASM, changed copy_high() to copy_linear(), so I can
  copy FROM extended memory as well as TO extended memory
  (mainly for debugging purposes)
- Now setting kernel command line to GRUB-style kernel pathname
- Now setting module command lines to GRUB-style module pathname
- Now setting boot drive field in Multiboot info structure

Version 0.4, released Aug 13, 2003

================================================================
Build
================================================================
You need MS-DOS or FreeDOS, you need NASM:
	http://nasm.sourceforge.net

and you need one of these 16-bit DOS compilers:
	Turbo C++ 1.0		http://bdn.borland.com/museum/
	Turbo C++ 3.0           (pay-ware)
	Borland C++ 3.1		(pay-ware)

This code may not work when compiled with Turbo C++ 3.0
because of 'huge' pointer bugs.

Near the top of file MAKEFILE, check that the definition of
TCDIR is correct, and set the CC macro to either "tcc" or "bcc".
Then type
	make

If you don't like Make, modify the batch file BUILD.BAT and run
that instead.

Run MBLOAD.EXE from the MS-DOS or FreeDOS prompt.

I tried compiling with Watcom C but it doesn't work and I don't
know why.

================================================================
Features
================================================================
5 supported kernel formats:
	- Multiboot-kludge	- Multiboot-ELF
	- DJGPP COFF		- plain (non-Multiboot) ELF
	- Win32 PE COFF

6 methods of determining RAM size:
	- XMS			- INT 15h AX=E820h
	- INT 15h AX=E801h	- INT 15h AH=88h
	- INT 12h		- INT 21h AH=48h

3 methods of allocating memory:
	- XMS			- "raw"
	- INT 21h AH=48h

2 methods of entering protected mode:
	- INT 15h AH=89h	- "raw"

2 methods of copying data to extended memory:
	- INT 15h AH=87h	- "raw"

5 methods of enabling A20 gate:
	- XMS			- PS/2 (INT 15h AX=2401h)
	- fast (I/O port 92h)	- AT
	- Vectra

================================================================
Missing features
================================================================
The following Multiboot/GRUB features are not (yet) supported:
- Additional parameters on the kernel and module command lines
- GZIP compression
- GRUB config file (MENU.LST)
- GRUB boot menu
- Some fields in multiboot_info_t structure:
	- Drive info		- APM table
	- Symbol/section table	- Video info

I will probably not support these GRUB features:
- Stand-alone boot that does not require DOS
- Non-Multiboot 32-bit kernel formats supported by GRUB
  (e.g. Linux and BSD kernels)
- Network booting

VCPI and everything associated with it (EMM386, UMBs, EMS) is
obsolete, so I don't support that either. You can't start a
pmode kernel with this loader if EMM386 is present. If this is a
problem for you, let me know, and I'll try to add VCPI support.

================================================================
Compatible kernels
================================================================
32-bit protected mode kernels will be loaded in extended memory
(at or above 1 meg). The load address varies:
- Multiboot-ELF: Final load address comes from the ELF segment
  (program header) with the lowest LMA. If the Multiboot
  kludge fields are present in an ELF kernel, they will be
  ignored. (This is confusing, but it's what GRUB does.)
- Multiboot-kludge: Final load address comes from load_addr
  field of multiboot_header_t struct
- Non-Multiboot ELF, DJGPP COFF, Win32 PE COFF:
  Final load address is 1 meg (0x100000)

If XMS is present, the kernel and modules will initially be
loaded into whatever free XMS memory is available. The kernel
and modules will be copied to their final load address just
before entering protected mode.

On entry to the pmode kernel:

1. CPU is in 32-bit protected mode
2. CS points to a code segment descriptor with base address 0
   and no limit (i.e. limit = 4 gig - 1)
3. DS, SS, ES, FS, and GS point to an expand-up data segment
   descriptor with base address 0 and no limit (i.e. limit =
   4 gig - 1)
4. The size and location of the GDT and selector values are
   undefined. Your kernel should create it's own GDT as soon
   as possible.
5. Interrupts are disabled. No IDT is defined.
6. Paging is disabled. No page tables are defined.
7. EAX=0x2BADB002
8. EBX points to the multiboot_info_t struct, which contains
   system and module info. MBLOAD does not support every field
   of this struct that GRUB does.
9. A20 is enabled

Dynamically-linked or relocatable kernels are not supported.
(Base relocations in Win32 PE COFF executables will be ignored.)

Module files may be in any file format. They will be loaded
into memory in their entirety. Modules associated with a 32-bit
pmode kernel will be loaded into extended memory, just beyond
the kernel, on page (4K) boundaries. If the kernel is copied
downward to address 1 meg, the modules will also be copied
downward. This copying is transparent to the user -- module
addresses in the multiboot_info struct will be set correctly.

================================================================
Differences from GRUB
================================================================
- Adjacent memory fields of the same type returned by
  INT 15h AX=E820h are coalesced (e.g. unused EBDA)

- You get an INT 15h AX=E820h-style memory map even if the BIOS
  does not support this interrupt

- No support for chainload bootsectors. DOS hooks a lot of the
  real-mode interrupt vectors, making it nearly impossible to
  load a different real-mode OS after DOS has started. (Unless
  the other real-mode OS was designed with this in mind...)

================================================================
Yet to do
================================================================
- re-test everything (see matrix below)

- test extended memory sizing code on 486SX system. INT 15h
  AX=E820h and INT 15h AX=E801h are not supported on that
  system; and INT 15h AH=88h will return 0 if XMS is present

- if virtual addresses are used in the Multiboot kludge fields
  (e.g. 0xC0000000), will MBLOAD-C catch this error?

- check: is entry point of non-Multiboot file always a virtual
  address?

- copy_int15 doesn't work for me, so I disabled it.
  Maybe it fails when src and dst overlap...?
  copy_int15 rounds count down -- is this the problem?

================================================================
Baffling DPMI bug
================================================================
Attempt to load Mobius kernel after running a DPMI program
(DJGPP, DOOM) causes PC to reset.

Can't check this in Bochs because DPMI programs make Bochs crash.

Things that didn't help:
- LGDT with O32 prefix
- CLTS instruction just before entering pmode
- Create an IDT with handlers for the first 32 exceptions
- LIDT with O32 prefix
- Zeroing the NT and IOPL bits in EFLAGS
- Load 0x01 into CR0 register instead of OR'ing with 0x01
  (clears PG bit, ET, TS, EM, and MP)
- Load 0 into CR3 after entering pmode, to ensure TLB is flushed

Things yet to try:
- Load seg regs with 16-bit values instead of 32-bit
- Create and load a TSS, with proper (?) I/O permission bitmap

================================================================
Test matrix
================================================================
108 cases to test (3 x 36):

hardare/OS: Pentium PC/MSDOS 7.0
        kernel uses
        virtual         kernel          Multi-
        addresses?      file format     boot?   XMS     works?
        ----------      -----------     ------  ----    ------
        (yes/no)        (DJGPP COFF,    (yes/   (none,  x
                        Win32 PE COFF,  no)     HIMEM,
                        ELF)                    XMSMMGR)

hardare/OS: 486SX/MSDOS 7.0
        kernel uses
        virtual         kernel          Multi-
        addresses?      file format     boot?   XMS     works?
        ----------      -----------     ------  ----    ------
        (yes/no)        (DJGPP COFF,    (yes/   (none,  x
                        Win32 PE COFF,  no)     HIMEM,
                        ELF)                    XMSMMGR)

hardare/OS: Bochs/FreeDOS
        kernel uses
        virtual         kernel          Multi-
        addresses?      file format     boot?   XMS     works?
        ----------      -----------     ------  ----    ------
        (yes/no)        (DJGPP COFF,    (yes/   (none,  x
                        Win32 PE COFF,  no)     FDXMS,
                        ELF)                    XMSMMGR)

================================================================
To do later
================================================================
- GZIP compression (ugh)
- support multiple user interfaces: none, command-line (GRUB),
  menu (GRUB), dialog (BING), COMMAND.COM, bash
- support real-mode (16-bit) kernels
	- DOS .EXE files, with or without relocations
        - DOS .COM files, including .COM files larger than 64K
	- chain-loaded bootsectors

16-bit kernels will be loaded into conventional memory on a
paragraph (16-byte) boundary. On entry to the 16-bit kernel:
1. CS=DS=SS=FS=GS. The exact value varies.
2. IP=0x100. The loader assumes that the 16-bit kernel is
   ORG 100h, like a .COM file. Unlike a .COM file, the kernel
   may be larger than 64K.
3. DX:AX=0x2BADB002
4. ES:BX points to the multiboot_info struct, which contains
   system and module info.
5. Interrupts are disabled.

Modules associated with a 16-bit real mode kernel will be
loaded into conventional memory, just beyond the kernel,
on paragraph boundaries.

The following fields are 16:16 far pointers instead of
32-bit near linear pointers for 16-bit kernels:
	multiboot_info_t.cmdline
	multiboot_info_t.mods_addr
	multiboot_info_t.mmap_addr
	module_t.mod_start
	module_t.mod_end
	module_t.string (module command-line)

Segment part of each address must be identical, e.g. FP_SEG(
multiboot_info_t.cmdline) == FP_SEG(multiboot_info_t.mod_end)

================================================================
Pseudocode
================================================================
kernel		entry	section
file format	point	adrs	kernel physical (load) address
-------------	-----	-------	--------------------------------
Multiboot-ELF   phys    virtual lowest segment (program hdr) adr
M'boot-kludge	phys	phys	from kludge fields
ELF		virtual	virtual	=1 meg
DJGPP COFF	virtual	virtual	=1 meg
Win32 PE COFF	virtual	virtual	=1 meg

to validate kernel file format and scan sections/segments:
- seek to start of opened kernel file
- read file headers; return +1 if short read
- validate file headers; return +1 if not desired format
- set global fields
	- g_pmode	=1 if 32-bit pmode kernel
	- g_virt_entry	=1 if entry point is a virtual addresses
	- g_entry	= kernel entry point
	- g_phys	= kernel physical (load) address
	- g_krnl_format	= pointer to kernel file format string
- for each section/segment in kernel file:
	- read section/segment, error if short read and I/O error
	- error if invalid, illegal, unsupported, or broken section/segment
	- store section name, address, size, file offset, and flags
	- split off BSS section from data section, if necessary

to load kernel after validating it and scanning sections/segments:
- scan kernel file sections, find highest and lowest addresses
- convert highest address to extent (size) by subtracting lowest address
- allocate memory block (extended/XMS memory for pmode kernel,
  conventional memory for 16-bit kernel), save block address == g_linear
- convert lowest address to virt-to-phys by subtracting it from g_linear
- for each section
	- if SF_LOAD
		- seek to sect->offset
		- load sect->size bytes to sect->adr + virt-to-phys
	- if SF_ZERO
		- zero sect->size bytes at sect->adr + virt-to-phys
- if kernel entry point is a virtual address, convert it to physical
  now by adding (final_load_address - lowest_virtual_address)

before entering pmode:
- if g_linear != g_phys
	- disable interrupts
	- copy (g_extmem_adr - g_linear) bytes from g_linear to g_phys

================================================================
Bootloader flowchart
================================================================
1.   check for DOS
2.   check for 32-bit CPU
3.   if /*DOS and*/ 32-bit CPU, check for V86 mode
4.   /*if DOS,*/ check for XMS

5.   if 32-bit CPU, use INT 15h AX=E820h to get memory map
6.   if INT 15h AX=E820h fails, use INT 12h to get conventional
     memory size and store in map
7.   if INT 15h AX=E820h fails, use INT 15h AX=E801h to get
     extended memory size and store in map
8.   if INT 15h AX=E801h fails, use INT 15h AH=88h to get
     extended memory size and store in map
9.   if INT 15h AH=88h fails or returns 0, read extended
     memory size from CMOS and store in map

10.  if DOS, use INT 21h AH=48h to allocate largest
     conventional memory block
     else scan memory map to find largest conventional memory block
     (excluding conventional memory used by the loader itself)

11.  if XMS:
11a. get XMS driver entry point
11b. get size of largest free XMS block
11c. allocate largest free XMS block
11d. lock XMS block
11e. adjust block address and size to page (4K) limits
11f. set XMS-in-use flag
     else (no XMS) scan memory map to find largest extended memory block

12.  display system info: memory ranges, conventional and extended/XMS
     memory sizes and addresses, DOS, 32-bit CPU, V86 mode

13. scan the DRIVER.SYS device chain (INT 2Fh AX=0803h),
    loading hard drive partition tables as necessary,
    to associate DOS drive letters to INT 13h drive numbers
    and partition numbers

14. get kernel name from command-line

15. use TRUENAME to get canonical DOS full path to kernel,
    then process path into GRUB format (GRUB device name
    instead of DOS drive letter, forward slashes instead
    of backward, lower-case names). Store full kernel path
    in Multiboot info structure.

16. open kernel file

17.  if kernel contains valid Multiboot header and is ELF:
17a. set pmode kernel = true
17b. validate ELF file headers
17c. get VIRTUAL entry point
17d. get sizes, file offsets, and VIRTUAL addresses of ELF segments
     (program headers)
	- choke on DYNAMIC and SHLIB segments
	- make sure each segment has the same virt-to-phys value
	- get lowest segment physical address (LPA)
17e. set load adr = LPA
17f. set virtual addresses = true
17g. warn if Multiboot "aout kludge" fields are present

18.  if kernel contains a valid Multiboot header and is NOT ELF:
18a. set pmode kernel = true
18b. verify that "aout kludge" is present
18c. get PHYSICAL entry point from kludge
18d. get size, file offset, and PHYSICAL address of text-and-data section
18e. get size and PHYSICAL address of BSS section
18f. get load adr from kludge
18g. set virtual addresses = false

19.  if kernel is ELF (not Multiboot)
19a. set pmode kernel = true
19b. validate ELF file headers
19c. get VIRTUAL entry point
19d. get sizes, file offsets, and VIRTUAL addresses of ELF segments
     (program headers)
	- choke on DYNAMIC and SHLIB segments
19e. set load adr = 1 meg
19f. set virtual addresses = true

20.  if kernel is DJGPP COFF (not Multiboot)
20a. set pmode kernel = true
20b. validate COFF file headers
20c. get VIRTUAL entry point
20d. get sizes, file offsets, and VIRTUAL addresses of sections
20e. set load adr = 1 meg
20f. set virtual addresses = true

21.  if kernel is Win32 PE COFF (not Multiboot)
21a. set pmode kernel = true
21b. validate DOS .EXE and Win32 PE COFF file headers
21c. get VIRTUAL entry point
21d. get sizes, file offsets, and VIRTUAL addresses of sections
	- choke on .idata section
21e. split off BSS section from .data, if necessary
21f. set load adr = 1 meg
21g. set virtual addresses = true

22.  if kernel has unknown file format, say so now

23.  display kernel file name, file format, and entry point
24.  dump kernel sections

25.  make sure kernel load adr is at or above 1 meg
26.  get kernel extent (size)
27.  allocate memory for kernel
28.  read or zero kernel sections
29.  adjust kernel entry point if it's a virtual address

30.  handle errors: unexpected EOF, too many sections/segments
     in kernel file, kernel file is dynamically-linked,
     invalid ELF kernel, invalid Multiboot header (not ELF
     nor kludge), out of conventional/extended/XMS memory

31.  close kernel file
32.  if pmode kernel
32a. error if V86 mode
32b. error if not 32-bit CPU

33.  for each module name on command line
33a. print warning and break from loop if too many modules

33b. Use TRUENAME to get canonical DOS full path to module,
	then process path into GRUB format (GRUB device name
	instead of DOS drive letter, forward slashes instead
	of backward, lower-case names). Store full module path
	in Multiboot module structure.

33c. open module file (error if can't)
33d. allocate memory for module
33e. load entire module into memory
33f. close module file
33g. display module info

34. get current DOS drive, convert to GRUB device name,
    store in Multiboot header

35. init remaining fields of Multiboot info struct:
    conventional and extended memory sizes, locations
    of module table, BIOS memory map, and ROM config table,
    and loader name

36. turn off floppy motors

37. prompt user to abort or continue

38. enter_pmode:
38a. if necessary, copy kernel and modules to their final load address
38b. if necessary, adjust module addresses in Multiboot module table
38c. use INT 15h AH=89h to enter pmode
38d. if INT 15h AH=89h fails, use "raw" method of entering pmode
