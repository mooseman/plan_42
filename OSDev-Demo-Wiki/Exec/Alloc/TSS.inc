;
; Exec/Alloc/TSS.inc
;

; This module provides functions to allocate and configure Task State Segments.

;-------------------------------------------------------------------------------
; This function allocates sufficient RAM for a TSS, and fills in the most
; important entries: EFlags, IOMap and LDT.
;
; It then returns a Data descriptor, ready for more customisation. To actually
; turn it into a TSS, call Exec.Alloc.TSS.Enable.
;
; Input:  AX  = LDT to use for the TSS
; Output: AX  = GDT Descriptor for (future) TSS, or zero on error
;         ES  = GDT Descriptor unless AX is zero
;         EAX = Memory Base
;         ECX = Memory Size
Exec.Alloc.TSS:

Exec.TSS.EFlags EQU             x86.EFlags.IF | x86.EFlags.1 ; Enable interrupts

%push Alloc.TSS ; Let's not leave these %defines just lying around...
%define         %$LDT           EBP - 4
%define         %$Base          EBP - 8
%define         %$Size          EBP - 12

                ENTER           12, 0

                MOV             [%$LDT],EAX

                MOV             ECX,x86.TSS_size ; Size to allocate
                CALL            Exec.Alloc.RAM
                TEST            EAX,EAX          ; Any left?
                JZ              .End             ; Pity that!

                MOV             [%$Base],EAX
                MOV             [%$Size],ECX

                MOV             DL,Type.Mem(Data, DPL0, RW)
                MOV             DH,Gran.Mem(Byte, Small)
                CALL            Exec.Alloc.GDT.Mem ; Get Descriptor to use
                TEST            EAX,EAX          ; Any left?
                JZ              .End             ; Pity that! **** Memory leak! ****

                MOV             ES,AX            ; Fresh Descriptor!
                MOV             ECX,[%$Size]     ; Number bytes to Zero
                XOR             EDI,EDI          ; Zero everything
                CALL            Exec.Alloc.RAM.Zero

                MOV             AX,[%$LDT]
                MOV             ECX,[%$Size]
                MOV             [ES:x86.TSS.LDT],AX
                MOV             [ES:x86.TSS.IOMap],CX
                MOV    DWORD    [ES:x86.TSS.EFlags],Exec.TSS.EFlags

                MOV             EAX,[%$Base]
.End:
                LEAVE
%pop
                RET

;-------------------------------------------------------------------------------
; This function converts a Segment previously allocated with Alloc.TSS (above)
; into a true TSS. Note that as soon as this happens, the TSS is a candidate for
; switching.
;
; Input:  ES  = Descriptor to modify
;         AH  = System.TSS if TSS is to be a System TSS. Zero otherwise.
; Output: EBX = TSS Descriptor
;         ES modified
Exec.Alloc.TSS.Enable:

; Note that I could use GS: overrides to access the GDT, but ES points to the
; soon-to-be TSS - a Bad Thing. This fixes that!
                MOV             EBX, ES         ; Get Descriptor to modify

                PUSH            GS              ; Get GDT into ES
                POP             ES

                MOV             [ES:EBX+x86.Desc.Type],BYTE Type.Sys(TSS, DPL0, 386)
                OR              [ES:EBX+x86.Desc.Mem.Gran],AH ; OR in System.TSS

                RET
