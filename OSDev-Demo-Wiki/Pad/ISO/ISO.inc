;
; Pad/ISO/ISO.inc
;

; This file performs the necessary binary padding to generate a .iso file.
;
; To make an ISO bootable, there needs to be some sectors in defined locations:
; Primary Volume; Boot Volume; Terminator Volume; Boot Catalog; Little-endian
; Path Table; Big-endian Path Table, and finally the Root directory.
; They've been included below in the correct order.
;
; Note that the Boot Catalog can advise the BIOS that it wants to emulate a
; HardDisk, a Floppy, or nothing: quite frankly, this image doesn't care.
;
; Note that according to the original standard, certain strings are VERY
; constrained. These constraints have been relaxed over the years: I'm taking
; advantage. To become more compliant, you will need to restrict your strings.
;
; The biggest limitation to the technique used here is that it will only work if
; the previous image that this module is padding onto is no more than 32 kiB in
; size. This value is defined in ISO.Start.Size

                SEGMENT         ISO

ISO.Timezone    EQU                     40      ; GMT+10:00 (AEST)

%include        "Pad/ISO/Defn.inc"

%assign         ISO.Sectors     16

ISO.Padding     EQU             ISO.Sectors*ISO.Sector.Size - ISO.Start.Size

TIMES           ISO.Padding     DB 00h

%include        "Pad/ISO/Primary.inc"

%include        "Pad/ISO/Boot.inc"

%include        "Pad/ISO/Terminator.inc"

%include        "Pad/ISO/Catalog.inc"

%define         ISO.Endian      Little
%include        "Pad/ISO/PathTable.inc"
%undef          ISO.Endian

%define         ISO.Endian      Big
%include        "Pad/ISO/PathTable.inc"
%undef          ISO.Endian

%include        "Pad/ISO/RootDir.inc"

ISO.Sector.Last      EQU        ISO.Sectors

ISO.Size        EQU             $ - $$
