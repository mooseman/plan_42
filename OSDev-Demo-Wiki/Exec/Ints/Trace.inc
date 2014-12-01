;
; Exec/Ints/Trace.inc
;

; This module installs a Single-Step (Trace) handler
Exec.Ints.Trace:
                MOV             EAX,Ints.Single
                MOV             EBX,IDT.Single
                MOV             CX,Selector(GDT.Ints, GDT, RPL0)
                MOV             DL,Type.Sys(Trap, DPL0, 386)
                CALL            Exec.Alloc.IDT
                RET
