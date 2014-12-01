;
; Demo.asm
;

%define         Version.Program   "Demonstrator"
%define         Version.Author    "John Burger"
%define         Version.Name      Version.Author, "'s 80386 ", Version.Program
%define         Version.Copyright "(c)2014"

%define         Version.Major   1
%define         Version.Minor   0
%define         Version.Build   1000

%defstr         Version.String  %[Version.Major].%[Version.Minor].%[Version.Build]

;*******************************************************************************

; This file is a complete example of the steps necessary to get a '386 (or
; later) PC to boot into Protected Mode, start some Tasks, and switch between
; them. It is completely written in assembly language, and uses no extra
; libraries.
;
; It has been written to be assembled with the Netwide Assembler NASM, which
; is a free download (http://www.nasm.us/) and will run on a number of different
; development platforms - Windows, Linux and OS-X.
;
; The NASM command line to assemble this source depends on the desired format:
; For CD:        nasm -o Demo.iso Demo.asm
; For USB:       nasm -o Demo.usb Demo.asm
; For Floppy:    nasm -o Demo.flp Demo.asm
; For Hard Disk: nasm -o Demo.bin Demo.asm
;
; Because NASM (by default) is a linear assembler, it is easy to use it to
; produce binary output. And since most Disk Image file formats are straight-
; forward binary, it is also easy to get NASM to produce these formats. Finally,
; most Disk Image formats can use the same binary, with extra 'decorations',
; so the same output can be used for multiple formats - simultaneously!
;
; By default, the binary output for this program can be used for any standard
; disk format - Floppy, USB, CD or even Hard Drive. You have two options;
; 1) Write it to physical media. Not as a file, but at a sector-by-sector level
;    starting at the first sector, using some form of low-leve drive writing
;    program (dd under Linux). Needless to say, this will effectively DESTROY
;    the existing data on the media! The easiest is probably to produce an ISO
;    file and use an ISO Burner to burn it to a CD.
;    Note that some USB sticks, and all hard drives, have a Partition Table
;    which divides the Drive into one or more Volumes. For these, you should
;    write the output to the sectors at the start of the Volume, not the Drive.
;    You can write it to the beginning of the Drive, but you'll DESTROY the
;    defined Partitions and everything in them!
; 2) Give the output file an appropriate extension (.flp, .img or .iso) and use
;    the output as a Floppy or CD stand-in with a virtualisation program like
;    VMware. Note that in this case, the output cannot be used for a Hard Drive:
;    it is not in the .vmdk or .vdi form that these programs use.
;===============================================================================

; Define .map file output. Map files are your friend! They can help you work out
; whether the assembler understands what you thought you told it...
                [map all Demo.map]

; You can tailor the output (make it smaller) by modifying these %defines,
; depending on just what the final desired output format will be. See the usage
; of the IMAGE.* defines at the bottom of this file.
%define         IMAGE.ISO       ; Write as bootable CD (ISO-9660 and El Torito)
;%define         IMAGE.FLOPPY    ; Write as full Floppy image

%assign         Demo.Size       0     ; Starting size. See Sizes.inc below

;===============================================================================

; The following are just definitions. Lots and lots of definitions...
; I hate "magic" numbers. Only 0 and 1 are numbers; the rest need labels!
; And comments. Lots and lots of comments...
%include        "x86/x86.inc"   ; Definitions for CPU
;===============================================================================
%include        "BIOS/BIOS.inc" ; Definitions for BIOS
;===============================================================================
%include        "Dev/Dev.inc"   ; Definitions for other Devices
;===============================================================================
%include        "Demo.inc"      ; Definitions for the rest of the program

;*******************************************************************************

; This is the BIOS Boot entry point. It runs in Real Mode, and assumes the
; standard BIOS Boot Specification 1.01.

                USE16           ; Start in 16-bit Real Mode

%include        "Boot/Boot.inc" ; Real Mode bootstrap

;*******************************************************************************

; From here down is Protected Mode code. It is loaded by the above Boot code
; to the defined location, "jettisoning" the above code as it's no longer needed

                USE32           ; The rest is 32-bit Protected Mode

%include        "Data.inc"      ; Global Data
;===============================================================================
%include        "Ints/Ints.inc" ; Interrupt handlers
;===============================================================================
%include        "User/User.inc" ; User Mode program
;===============================================================================
%include        "Exec/Exec.inc" ; Supervisor Mode executive

;*******************************************************************************

; Finally, Pad the assembled image to make it usable as a Disk Image file.

%include        "Pad/Sizes.inc"    ; Calculate the end-of-Segment sizes

;===============================================================================
; First, Pad it up to the next Hard Disk size.
%include        "Pad/HardDisk.inc" ; Perform padding to generate Hard Disk image

; Can now calculate the final Load Size
Load.Size       EQU             Demo.Size - Real.Size

; See Boot/Load.inc for why
%if Load.Size > Load.Max
%error "Demo image too large for loader!"
%endif

;===============================================================================
; For any other Disk Image format, extra work is needed.

; If you're going to write it to a Floppy, USB or Hard Disk, the following isn't
; required.
; If you want a CD-Rom image (.iso), you don't need the Floppy stuff.
; If you want a Floppy image (.flp or .img), you don't need the ISO stuff.
; Leaving them in won't hurt, it just may be a larger file with LOTS of 0s.

%ifdef IMAGE.ISO
%assign         ISO.Start.Size    Demo.Size
%include        "Pad/ISO/ISO.inc" ; Perform padding to generate ISO image
%assign         Demo.Size         Demo.Size + ISO.Size
%endif ; IMAGE.ISO

;===============================================================================
%ifdef IMAGE.FLOPPY
%include        "Pad/Floppy.inc" ; Perform padding for Floppy image
%endif ; IMAGE.FLOPPY
