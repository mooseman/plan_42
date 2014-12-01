;
; BIOS/Disk.inc
;

; These are the definitions for the BIOS Disk controller

BIOS.Disk.Sector.Size EQU       512

BIOS.Disk.Int      EQU          13h
BIOS.Disk.Fn.Reset EQU          00h
BIOS.Disk.Fn.Read  EQU          02h
