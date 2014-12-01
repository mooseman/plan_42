;
; Exec/Alloc/GDT.inc
;

; This module provides functions to allocate entries in the GDT.

;-------------------------------------------------------------------------------
; This function allocates a new Desc.Sys in the GDT
; Input:  CX:EAX = Selector : Offset of handler
;         DL     = Descriptor Type
; Output: EAX    = Allocated Descriptor, or zero on error
;         EAX, EBX, EDI modified

Exec.Alloc.GDT.Sys:
                MOV             DI,GS           ; Get GDT selector
                MOV             ES,DI           ; Into ES for Alloc.DT

                PUSH            EAX             ; Save for later
                CALL            Exec.Alloc.DT   ; Allocate an Entry
                MOV             EBX,EAX         ; This is Descriptor to modify

                TEST            EAX,EAX         ; Any left?
                POP             EAX             ; It's later!
                JZ              .Finish         ; Uh oh!

                CALL            Exec.Alloc.DT.Sys ; Call helper function

                LGDT            [ES:GDT.Alloc.Pseudo] ; Update GDT size
.Finish:
                MOV             EAX,EBX         ; Return value
                RET

;-------------------------------------------------------------------------------
; This function allocates a new Desc.Mem in the GDT
; Input:  EAX    = Base
;         ECX    = Size (Converted to Limit)
;         DL     = Descriptor Type
;         DH     = Descriptor Granularity
; Output: EAX    = Allocated Descriptor, or zero on error
;         ECX, DH, EDI modified

Exec.Alloc.GDT.Mem:
                MOV             DI,GS           ; Get GDT selector
                MOV             ES,DI           ; Into ES for Alloc.DT

                PUSH            EAX             ; Save for later
                CALL            Exec.Alloc.DT   ; Allocate an Entry
                MOV             EBX,EAX         ; This is Descriptor to modify

                TEST            EAX,EAX         ; Any left?
                POP             EAX             ; It's later!
                JZ              .Finish         ; Uh oh!

                CALL            Exec.Alloc.DT.Mem ; Call helper function

                LGDT            [ES:GDT.Alloc.Pseudo] ; Update GDT size
.Finish:
                MOV             EAX,EBX         ; Return value
                RET
