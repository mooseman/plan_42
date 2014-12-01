;
; Data.inc
;

; This is the definition for the global Data Segment. Note that it is
; mostly populated at Boot by Boot/RAM.inc

                 SEGMENT        Data  START=0  ALIGN=16  NOBITS

                 USE32

Data.RAM         RESD           1       ; Beginning of available RAM
Data.RAMTop      RESD           1       ; Top of RAM as reported by BIOS

Data.Key.Code    RESB           1       ; Storage for key scan-code
Data.Key.Shifts  RESB           1       ; Storage for current Key Shift states

                 RESB           12      ; Reserved

Data.RAMMap.Num  RESW           1       ; Number of BIOS.RAMMap entries

Data.RAMMap      RESB           Data.Size - ($-$$) ; RAMMap
Data.RAMMap.Size EQU            $ - Data.RAMMap
Data.RAMMap.Max  EQU            Data.RAMMap.Size / BIOS.RAMMap_size
