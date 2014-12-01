;
; Exec/Ints/NoSeg.inc
;

; This module installs an example No Segment handler.
Exec.Ints.NoSeg:
                MOV             EAX,Ints.NoSeg
                MOV             EBX,IDT.NoSeg
                MOV             CX,Selector(GDT.Ints, GDT, RPL0)
                MOV             DL,Type.Sys(Trap, DPL0, 386)
                CALL            Exec.Alloc.IDT
                RET
