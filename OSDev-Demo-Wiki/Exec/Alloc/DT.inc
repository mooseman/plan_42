;
; Exec/Alloc/DT.inc
;

; This module provides generic functions for any of the Descriptor Tables:
; GDT, LDT or IDT. Only the first two have the capacity for new Descriptors
; to be added though.

;-------------------------------------------------------------------------------
; This function allocates a new Descriptor in either the GDT or LDT. It uses the
; same algorithm in both cases, since they've been set up with similar Allocator
; Data in their first entry (which is not used in the GDT, but is a small waste
; for an LDT. Ah well!)
;
; The algorithm is simple:
; 1) If the .Free entry is non-zero, that will be the returned Descriptor.
;    First though, load the .Free's '.Free' entry, since that is the next
;    available entry (could be zero), and store that in .Free instead.
; 2) If the .Free entry is zero, then check .Limit. If that is -1, the table is
;    FULL! Return a zero Descriptor.
; 3) Otherwise, add Descriptor size to .Limit and return the old value - don't
;    forget to add 1 though, since it was a Limit.
;
; Input:  ES  = Descriptor Table to allocate from
; Output: EAX = Descriptor allocated, or zero if Table is full
;         EDI modified

Exec.Alloc.DT:
                MOVZX           EAX,WORD [ES:DT.Alloc.Free] ; Get potential Free one
                TEST            EAX,EAX                 ; Any?
                JZ              .Grow                   ; No, so grow DT

                MOVZX           EDI,WORD [ES:EAX + DT.Alloc.Free] ; Get Free's next
                MOV             [ES:DT.Alloc.Free],EDI  ; Store as new Free
                JMP             .End                    ; Done!
.Grow:
                MOV             AX,[ES:DT.Alloc.Limit]  ; Get current Limit
                INC             AX                      ; Add one
                JZ              .End                    ; Uh oh! Full!

                ADD     WORD    [ES:DT.Alloc.Limit],x86.Desc_size ; Increase Limit
.End:
                RET

;-------------------------------------------------------------------------------
; This function modifies an existing DT entry to become a Desc.Sys.
; NOTE THAT ANY SEGMENT REGISTER CONTAINING THIS DESCRIPTOR WILL CAUSE A FAULT!
; Although a Fault is only generated when a Descriptor is loaded, that is
; PRECISELY what happens during the return from an interrupt!
; In other words, it's NOT safe to keep it, even if "only for a little while..."
; Input:  ES:EBX = Descriptor Table entry to modify
;         CX:EAX = Selector : Offset of System entry
;         DL     = Descriptor Type
; Output: EAX modified

Exec.Alloc.DT.Sys:
                MOV             [ES:EBX + x86.Desc.Sys.OffsetLo],AX
                SHR             EAX,16
                MOV             [ES:EBX + x86.Desc.Sys.Selector],CX
                MOV             [ES:EBX + x86.Desc.Sys.OffsetHi],AX
                MOV             [ES:EBX + x86.Desc.Sys.Type],DL

                RET

;-------------------------------------------------------------------------------
; This function modifies an existing DT entry to become a Desc.Mem.
; Any Segment register containing the old value needs to be refreshed.
; Note that an interrupt WILL re-load a segment register: you cannot safely
; assume that a "stale" Segment register still uses the old data!
; Input:  ES:EBX = Descriptor Table entry to modify
;         EAX    = Base
;         ECX    = Size (Converted to Limit)
;         DL     = Descriptor Type
;         DH     = Descriptor Granularity
; Output: EAX, ECX and DH modified

Exec.Alloc.DT.Mem:
                TEST            DL,x86.Desc.Mem.Type.Code
                JNZ             .Scale          ; Code cannot be Expand-Down
                TEST            DL,x86.Desc.Mem.Type.Expand
                JZ              .Scale          ; Not Expand-Down either
                ADD             EAX,ECX         ; It is! Add Size to Base
                NEG             ECX             ; And negate ECX to exclude size
                TEST            DH,x86.Desc.Mem.Gran.Big ; 32-bit?
                JNZ             .Scale          ; Yes
                AND             ECX,0000_FFFFh  ; No. Bring down to 16-bit range
                SUB             EAX,0001_0000h  ; And alter Base to suit
.Scale:
                DEC             ECX             ; Convert to Limit
                CMP             ECX,000F_FFFFh  ; Will Limit fit?
                JBE             .Set            ; Yep, so do so
                SHR             ECX,12          ; Nope! Accommodate it
                OR              DH,x86.Desc.Mem.Gran.Gran
.Set:
                MOV             [ES:EBX + x86.Desc.Mem.LimitLo],CX
                SHR             ECX,16
                MOV             [ES:EBX + x86.Desc.Mem.BaseLo], AX
                SHR             EAX,16
                MOV             [ES:EBX + x86.Desc.Mem.BaseMid],AL
                OR              DH,CL
                MOV             [ES:EBX + x86.Desc.Mem.Type],   DL
                MOV             [ES:EBX + x86.Desc.Mem.Gran],   DH
                MOV             [ES:EBX + x86.Desc.Mem.BaseHi], AH

                RET
