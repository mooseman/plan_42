;
; BIOS/MemMap.inc
;

; This is the Memory Map of when the BIOS first boots.

; You'll notice that the Stack is on top of the Interrupt Vector Table!
BIOS.Stack.Base EQU             0000h      ; BIOS Boot stack
BIOS.Stack.Size EQU             0400h

BIOS.BDA.Base   EQU             0400h      ; BIOS Data Area
BIOS.BDA.Size   EQU             0100h

BIOS.MBR.Base   EQU             0600h      ; BIOS Master Boot Record load area
BIOS.MBR.Size   EQU             0200h

BIOS.Entry      EQU             7C00h      ; BIOS Boot Entry point

BIOS.Sig.Value  EQU             0AA55h     ; BIOS Signature flag
BIOS.Sig.Pos    EQU             BIOS.Disk.Sector.Size - 2
