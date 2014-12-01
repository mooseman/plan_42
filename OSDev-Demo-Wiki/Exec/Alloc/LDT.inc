;
; Exec/Alloc/LDT.inc
;

; This module provides functions to create LDTs and allocate entries in them.

;-------------------------------------------------------------------------------
; This function:
; 1) Allocates the requested RAM for an LDT;
; 2) Fills in the first entry with LDT.Alloc;
; 3) Fills in the second entry with an alias for this LDT (useful for debugging)
; 4) Returns a Data Descriptor for the LDT, ready for more customisation.
; To actually turn it into an LDT, call Exec.Alloc.LDT.Enable.
;
; Input:  ECX = Size
; Output: AX  = GDT Descriptor for (future) LDT, or zero on error
;         ES  = GDT Descriptor unless AX is zero
Exec.Alloc.LDT:

%push Alloc.LDT ; Let's not leave these %defines just lying around...
%define         %$Base          EBP - 4
%define         %$Size          EBP - 8

                ENTER           8, 0

                MOV             [%$Size], ECX

                CALL            Exec.Alloc.RAM
                TEST            EAX,EAX          ; Any left?
                JZ              .End             ; Pity that!

                MOV             [%$Base],EAX

                MOV             EBX,EAX          ; Base to allocate
                MOV             DL,Type.Mem(Data, DPL0, RW)
                MOV             DH,Gran.Mem(Byte, Small)
                CALL            Exec.Alloc.GDT.Mem ; Get Descriptor to use
                TEST            EAX,EAX          ; Any left?
                JZ              .End             ; Pity that! **** Memory leak! ****

                MOV             ES,AX            ; Fresh Descriptor!
                MOV             ECX,[%$Size]     ; Bytes to zero
                XOR             EDI,EDI          ; Zero everything
                CALL            Exec.Alloc.RAM.Zero

                ; Set up first entry as an LDT.Alloc
                MOV     WORD    [ES:LDT.Alloc.Limit],x86.Desc_size - 1
                MOV     BYTE    [ES:LDT.Alloc.Type],x86.Desc.Sys.Type.LDT
                MOV             [ES:LDT.Alloc.GDT],ES

                ; Set up second entry as an alias for this very LDT
                MOV             EAX,[%$Base]            ; Pass Base in
                MOV             ECX,[%$Size]            ; Size
                MOV             DL,Type.Mem(Data, DPL0, RW)
                MOV             DH,Gran.Mem(Byte, Small)
                CALL            Exec.Alloc.LDT.Mem      ; Allocate LDT entry

                MOV             AX,ES                   ; Return value
.End:
                LEAVE
%pop
                RET
;-------------------------------------------------------------------------------
; This function converts a Segment previously allocated with Alloc.LDT (above)
; into a true LDT.
;
; Input:  ES = Descriptor to modify
; Output: ES, EBX modified
Exec.Alloc.LDT.Enable:

; Note that I could use GS: overrides to access the GDT, but ES points to the
; soon-to-be LDT - a Bad Thing. This fixes that!
                MOV             EBX, ES         ; Get Descriptor to modify

                MOV             AX,GS           ; Get GDT into ES
                MOV             ES,AX

                MOV     BYTE    [ES:EBX+x86.Desc.Type],Type.Sys(LDT, DPL0, 286)

                RET
;-------------------------------------------------------------------------------
; This function allocates a new Desc.Sys in an LDT
; Input:  ES     = LDT to modify
;         CX:EAX = Selector : Offset of handler
;         DL     = Descriptor Type
; Output: EAX    = Allocated Descriptor, or zero on error
;         EBX, ESI, EDI modified

Exec.Alloc.LDT.Sys:
                PUSH            EAX             ; Save for later
                CALL            Exec.Alloc.DT   ; Allocate an Entry
                MOV             EBX,EAX         ; This is Descriptor to modify

                TEST            EAX,EAX         ; Any left?
                POP             EAX             ; It's later!
                JZ              .Finish         ; Uh oh!

                CALL            Exec.Alloc.DT.Sys ; Call helper function

.Finish:
                MOV             EAX,EBX         ; Return value
                RET

;-------------------------------------------------------------------------------
; This function allocates a new Desc.Mem in an LDT
; Input:  ES     = LDT to modify
;         EAX    = Base
;         ECX    = Size (Converted to Limit)
;         DL     = Descriptor Type
;         DH     = Descriptor Granularity
; Output: EAX    = Allocated Descriptor, or zero on error
;         EBX, ECX, DH, ESI, EDI modified

Exec.Alloc.LDT.Mem:
                PUSH            EAX             ; Save for later
                CALL            Exec.Alloc.DT   ; Allocate an Entry
                MOV             EBX,EAX         ; This is Descriptor to modify

                TEST            EAX,EAX         ; Any left?
                POP             EAX             ; It's later!
                JZ              .Finish         ; Uh oh!

                CALL            Exec.Alloc.DT.Mem ; Call helper function

.Finish:
                MOV             EAX,EBX         ; Return value
                RET
