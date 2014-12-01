;
; Exec/Alloc/IDT.inc
;

; This module provides functions to modify the IDT.

;-------------------------------------------------------------------------------
; This function modifies an IDT entry.
; Input:  EBX    = IDT Descriptor entry
;         CX:EAX = Selector : Offset of handler
;         DL     = Descriptor Type
; Output: ES, EAX, EDI modified

Exec.Alloc.IDT:
                MOV             DI,Selector(Exec.LDT.IDT, LDT, RPL0)
                MOV             ES,DI           ; Point to IDT with ES

                CALL            Exec.Alloc.DT.Sys
                RET
