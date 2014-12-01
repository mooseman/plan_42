;
; Ints/NoSeg.inc
;

; This module gives an example of a "No Segment" Fault handler.
; Of course, a real handler would have to load the missing Segment from
; somewhere - this one merely assumes that we "forgot" to mark it as .Present,
; fixes it, and resumes.

Ints.NoSeg:                                      ; Oops, forgot to mark as Present!
;               PUSH            0                ; PUSH Error Code - no, already there!
                PUSH            EBX
                PUSH            DS

                MOV             BX,Selector(GDT.VGA, GDT, RPL0)
                MOV             DS,BX
                INC     BYTE    [0016h]          ; Dingle(tm) on screen - show we've done it1

                MOV             EBX,[ESP+8]      ; Get error code
                AND             EBX,x86.Seg.TI   ; Isolate which DT it's in
                OR              BX,DT.Alias      ; Same Alias position in all DTs
                MOV             DS,BX            ; Point to relevant DT Alias

                MOV             EBX,[ESP+8]      ; Get error code (again)
                AND             BX,x86.Seg.Index ; Ignore extra bits
                OR      BYTE    [BX+x86.Desc.Type],x86.Desc.Type.Present ; 16-bit index!

                POP             DS
                POP             EBX
                ADD             ESP,4           ; Skip Error code
                IRETD                           ; And restart instruction
