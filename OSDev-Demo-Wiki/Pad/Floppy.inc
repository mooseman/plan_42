;
; Pad/Floppy.inc
;

; This file performs the necessary padding to generate a .flp or .img file.
; All of the hard work has been done: all we need to do is generate
; sufficient extra bytes to make the resultant output file large enough to
; satisfy programs like earlier versions VMware.
; These programs use the file size to determine the size of the simulated floppy,
; and from there the geometry to use (number of sectors per track, number of
; tracks per side, etc.)

                SEGMENT         Floppy

;Floppy.Format   EQU             720
;Floppy.Format   EQU             1200
Floppy.Format   EQU             1440
;Floppy.Format   EQU             2880

Floppy.Padding  EQU             Floppy.Format*1024 - Demo.Size ; From kiB to bytes

TIMES           Floppy.Padding  DB 00h

Floppy.Size     EQU             $ - $$

%assign         Demo.Size       Demo.Size + Floppy.Size
