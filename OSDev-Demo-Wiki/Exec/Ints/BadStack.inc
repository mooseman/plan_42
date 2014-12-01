;
; Exec/Ints/BadStack.inc
;

; Ths module installs a Stack Fault handler. It's mostly a stub, leaving the
; hard work to the generic .Fault handler.
Exec.Ints.BadStack:
                MOV             EAX,IDT.BadStack ; This is the IDT Entry
                MOV             EBX,Exec.LDT.BadStack ; This is the LDT triple
                MOV             EDX,.Handler     ; This is the handler
                CALL            Exec.Ints.Fault  ; Set it up
                RET
.Handler:
;               PUSH   DWORD    0               ; Pseudo-fault code (not required)
                PUSH            12              ; Interrupt number
                MOV             EBX,Exec.LDT.BadStack.TSS ; For TSS Backlink
                CALL            Exec.Ints.Fault.Handler
                ADD             ESP,8           ; Get rid of above
                IRETD                           ; Return, but resumes here, so...
                JMP             .Handler        ; ...go back for more
