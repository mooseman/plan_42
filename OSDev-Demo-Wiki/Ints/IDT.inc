;
; Ints/IDT.inc
;

; This is the Interrupt Descriptor Table for the system.
; For each potential interrupt, it points to the relevant routine.
; Note that only the first 32 Intel-reserved interrupts, and the 16 Interrupt
; ReQuest IRQs, are set up. Although sufficient memory has been allocated for
; all 256 interrupts, the IDT is only initialised with these first 48.
;
; EDIT: I decided to add two more, to be called from User mode:
; * INT 48 is a User-mode HLT replacement;
; * INT 49 is a Yield call - go to next Task.

                SEGMENT         IDT  VSTART=0  ALIGN=16

                USE32

IDT.Trap        EQU             Type.Sys(Trap, DPL0, 386)
IDT.Int         EQU             Type.Sys(Int,  DPL0, 386)
IDT.User        EQU             Type.Sys(Trap, DPL3, 386)

IDT:
IDT.Divide      Desc.Sys        GDT.Ints, Ints.Int0, IDT.Trap     ; Int 00h
IDT.Single      Desc.Sys        GDT.Ints, Ints.Int1, IDT.Trap     ; Int 01h
IDT.NMI         Desc.Sys        GDT.Ints, Ints.Int2, IDT.Trap     ; Int 02h
IDT.Debug       Desc.Sys        GDT.Ints, Ints.Int3, IDT.Trap     ; Int 03h
IDT.INTO        Desc.Sys        GDT.Ints, Ints.Int4, IDT.Trap     ; Int 04h
IDT.BOUND       Desc.Sys        GDT.Ints, Ints.Int5, IDT.Trap     ; Int 05h
IDT.BadOp       Desc.Sys        GDT.Ints, Ints.Int6, IDT.Trap     ; Int 06h
IDT.No387       Desc.Sys        GDT.Ints, Ints.Int7, IDT.Trap     ; Int 07h
IDT.Double      Desc.Sys        GDT.Ints, Ints.Int8, IDT.Trap     ; Int 08h
IDT.Bad387      Desc.Sys        GDT.Ints, Ints.Int9, IDT.Trap     ; Int 09h
IDT.BadTSS      Desc.Sys        GDT.Ints, Ints.Int10, IDT.Trap    ; Int 0Ah
IDT.NoSeg       Desc.Sys        GDT.Ints, Ints.Int11, IDT.Trap    ; Int 0Bh
IDT.BadStack    Desc.Sys        GDT.Ints, Ints.Int12, IDT.Trap    ; Int 0Ch
IDT.GPF         Desc.Sys        GDT.Ints, Ints.Int13, IDT.Trap    ; Int 0Dh
IDT.Page        Desc.Sys        GDT.Ints, Ints.Int14, IDT.Trap    ; Int 0Eh
IDT.15          Desc.Sys        GDT.Ints, Ints.Int15, IDT.Trap    ; Int 0Fh
IDT.Float       Desc.Sys        GDT.Ints, Ints.Int16, IDT.Trap    ; Int 10h
IDT.Align       Desc.Sys        GDT.Ints, Ints.Int17, IDT.Trap    ; Int 11h
IDT.Machine     Desc.Sys        GDT.Ints, Ints.Int18, IDT.Trap    ; Int 12h
IDT.SIMD        Desc.Sys        GDT.Ints, Ints.Int19, IDT.Trap    ; Int 13h
IDT.VM          Desc.Sys        GDT.Ints, Ints.Int20, IDT.Trap    ; Int 14h
IDT.21          Desc.Sys        GDT.Ints, Ints.Int21, IDT.Trap    ; Int 15h
IDT.22          Desc.Sys        GDT.Ints, Ints.Int22, IDT.Trap    ; Int 16h
IDT.23          Desc.Sys        GDT.Ints, Ints.Int23, IDT.Trap    ; Int 17h
IDT.24          Desc.Sys        GDT.Ints, Ints.Int24, IDT.Trap    ; Int 18h
IDT.25          Desc.Sys        GDT.Ints, Ints.Int25, IDT.Trap    ; Int 19h
IDT.26          Desc.Sys        GDT.Ints, Ints.Int26, IDT.Trap    ; Int 1Ah
IDT.27          Desc.Sys        GDT.Ints, Ints.Int27, IDT.Trap    ; Int 1Bh
IDT.28          Desc.Sys        GDT.Ints, Ints.Int28, IDT.Trap    ; Int 1Ch
IDT.29          Desc.Sys        GDT.Ints, Ints.Int29, IDT.Trap    ; Int 1Dh
IDT.Security    Desc.Sys        GDT.Ints, Ints.Int30, IDT.Trap    ; Int 1Eh
IDT.31          Desc.Sys        GDT.Ints, Ints.Int31, IDT.Trap    ; Int 1Fh

IDT.Timer       Desc.Sys        GDT.Ints, Ints.IRQ0, IDT.Int      ; Int 20h
IDT.Key         Desc.Sys        GDT.Ints, Ints.IRQ1, IDT.Int      ; Int 21h
IDT.Cascade     Desc.Sys        GDT.Ints, Ints.IRQ2, IDT.Int      ; Int 22h
IDT.COM2        Desc.Sys        GDT.Ints, Ints.IRQ3, IDT.Int      ; Int 23h
IDT.COM1        Desc.Sys        GDT.Ints, Ints.IRQ4, IDT.Int      ; Int 24h
IDT.IRQ5        Desc.Sys        GDT.Ints, Ints.IRQ5, IDT.Int      ; Int 25h
IDT.Floppy      Desc.Sys        GDT.Ints, Ints.IRQ6, IDT.Int      ; Int 26h
IDT.Printer     Desc.Sys        GDT.Ints, Ints.IRQ7, IDT.Int      ; Int 27h
IDT.RTC         Desc.Sys        GDT.Ints, Ints.IRQ8, IDT.Int      ; Int 28h
IDT.IRQ9        Desc.Sys        GDT.Ints, Ints.IRQ9, IDT.Int      ; Int 29h
IDT.IRQ10       Desc.Sys        GDT.Ints, Ints.IRQ10, IDT.Int     ; Int 2Ah
IDT.IRQ11       Desc.Sys        GDT.Ints, Ints.IRQ11, IDT.Int     ; Int 2Bh
IDT.Mouse       Desc.Sys        GDT.Ints, Ints.IRQ12, IDT.Int     ; Int 2Ch
IDT.IRQ13       Desc.Sys        GDT.Ints, Ints.IRQ13, IDT.Int     ; Int 2Dh
IDT.IDE0        Desc.Sys        GDT.Ints, Ints.IRQ14, IDT.Int     ; Int 2Eh
IDT.IDE1        Desc.Sys        GDT.Ints, Ints.IRQ15, IDT.Int     ; Int 2Fh

IDT.Halt        Desc.Sys        GDT.Ints, Ints.Halt, IDT.User     ; Int 30h
IDT.Yield       Desc.Sys        GDT.Ints, Ints.Yield, IDT.User    ; Int 31h
