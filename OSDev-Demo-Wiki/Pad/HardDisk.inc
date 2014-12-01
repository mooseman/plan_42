;
; Pad/HardDisk.inc
;

; This file performs the necessary padding to generate a HardDisk image file.
; All of the hard work has been done: all we need to do is generate sufficient
; extra bytes to round the resultant output file to the next Sector Size.

                SEGMENT         HardDisk

; Take the current Size for the program, and round it UP to the next Sector.Size
HardDisk.Last   EQU             (Demo.Size + BIOS.Disk.Sector.Size - 1) & ~(BIOS.Disk.Sector.Size - 1)
HardDisk.Padding EQU            HardDisk.Last - Demo.Size

TIMES           HardDisk.Padding DB 00h

HardDisk.Size   EQU             $ - $$

%assign         Demo.Size       Demo.Size + HardDisk.Size
