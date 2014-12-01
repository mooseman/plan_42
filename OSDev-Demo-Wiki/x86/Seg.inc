;
; x86/Seg.inc
;

; These are the definitions for the Segment selector registers

x86.Seg.Index   EQU            1111_1111_1111_1000b  ; Index into Table
x86.Seg.TI      EQU            0000_0000_0000_0100b  ; Table Indicator (GDT/LDT)
x86.Seg.RPL     EQU            0000_0000_0000_0011b  ; Requested Privilege Level

x86.Seg.LDT     EQU            x86.Seg.TI       ; Add this in to every LDT seg
x86.Seg.GDT     EQU            0

x86.Seg.RPL3    EQU            0000_0000_0000_0011b
x86.Seg.RPL2    EQU            0000_0000_0000_0010b
x86.Seg.RPL1    EQU            0000_0000_0000_0001b
x86.Seg.RPL0    EQU            0000_0000_0000_0000b

; A helper macro, making it easy to set all the fields of a Selector.
; The parameters are: Table Index, GDT or LDT, and RPL0-3
%define         Selector(Index, TI, RPL) ( Index + (x86.Seg.%+ TI | x86.Seg.%+ RPL) )
