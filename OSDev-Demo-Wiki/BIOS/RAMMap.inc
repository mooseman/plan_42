;
; BIOS/RAMMap.inc
;

; These are the definitions for the BIOS RAM Map information handler

BIOS.RAMMap.Int EQU             15h
BIOS.RAMMap.Fn  EQU             0000_E820h
BIOS.RAMMap.Magic EQU           "PAMS"          ; NASM reverses "SMAP"

; This is the BIOS RAMMap structure
                STRUC           BIOS.RAMMap
.Base           RESQ            1
.Length         RESQ            1
.Type           RESD            1
.Flags          RESD            1 ; Added in later BIOSes
                ENDSTRUC

BIOS.RAMMap.Flags.DontIgnore EQU 0000_0000_0000_0001b
