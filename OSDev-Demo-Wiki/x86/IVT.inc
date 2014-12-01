;
; x86/IVT.inc
;

; These are the definitions for the (Real Mode) Interrupt Vector Table.

x86.Ints        EQU             256

x86.IVT.Base    EQU             0000h         ; Interrupt Vector Table
x86.IVT.Size    EQU             x86.Ints * 4  ; This many DWORD vectors
