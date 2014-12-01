;
; Exec/User/LDT.inc
;

; This module creates a User-mode Program's LDT and populates it.
; For demonstration purpose, it deliberately "forgets" to mark some of the
; Segments as "Present", which should trigger the Segment Not Present exception.

Exec.User.LDT:
                ; Allocate an LDT
                MOV             ECX,User.LDT.Size       ; Need this many bytes
                CALL            Exec.Alloc.LDT
                TEST            EAX,EAX                 ; Any left?
                JZ              .Failed                 ; Nope! Finished

                MOV             [%$LDT],EAX             ; Want to keep this!

                ; Allocate Stack0
                MOV             ECX,User.Stack0.Size    ; Get some RAM for Stack0
                CALL            Exec.Alloc.RAM
                TEST            EAX,EAX                 ; Any left?
                JZ              .Failed                 ; Nope!

                MOV             DL,Type.Mem(Stack, DPL0, RW)
                MOV             DH,Gran.Mem(Byte, Small)
                CALL            Exec.Alloc.LDT.Mem      ; Allocate LDT Entry
                CMP             EAX,User.LDT.Stack0
                JNE             .Failed

                ; Allocate Stack3
                MOV             ECX,User.Stack3.Size    ; Get some RAM for Stack3
                CALL            Exec.Alloc.RAM
                TEST            EAX,EAX                 ; Any left?
                JZ              .Failed                 ; Nope!

                MOV             DL,Type.Mem(Stack, DPL3, RW)
                MOV             DH,Gran.Mem(Byte, Small)
                CALL            Exec.Alloc.LDT.Mem      ; Allocate LDT Entry
                CMP             EAX,User.LDT.Stack3
                JNE             .Failed

                ; Allocate Data
                MOV             ECX,User.Data.Size      ; Get some RAM for Data
                CALL            Exec.Alloc.RAM
                TEST            EAX,EAX                 ; Any left?
                JZ              .Failed                 ; Nope!

                MOV             DL,Type.Mem(Data, DPL3, RW); & ~x86.Desc.Type.Present
                MOV             DH,Gran.Mem(Byte, Big)
                CALL            Exec.Alloc.LDT.Mem      ; Allocate LDT Entry
                CMP             EAX,User.LDT.Data
                JNE             .Failed

                ; Allocate Code
                MOV             EAX,User.Base           ; Assembled User code
                MOV             ECX,User.Size
                MOV             DL,Type.Mem(Code, DPL3, NoRW); & ~x86.Desc.Type.Present
                MOV             DH,Gran.Mem(Byte, Def32)
                CALL            Exec.Alloc.LDT.Mem      ; Allocate LDT Entry
                CMP             EAX,User.LDT.Code
                JNE             .Failed

                ; Now enable the LDT
                CALL            Exec.Alloc.LDT.Enable   ; Finished!

                MOV             EAX,[%$LDT]             ; LDT descriptor
                JMP             .End
.Failed:
                XOR             EAX,EAX
.End:
                RET
