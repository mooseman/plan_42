;
; Boot/CPU.inc
;

; Test whether CPU is a '386 (minimum)
Boot.CPU:

                CPU 8086                  ; Assume lowest possible option
; The 8086 pushes a different SP to the others
                PUSH            SP
                POP             AX

                CMP             SP,AX     ; Comparison designed to set Carry too
                JNE             $         ; Uh oh! ***STOP!***

                CPU 286                   ; At least we know it's this good!
; The '286 always clears IOPL
                PUSHF
                POP             AX

                OR              AX,x86.EFlags.IOPL

                PUSH            AX
                POPF

                PUSHF
                POP             CX

                CMP             CX,AX     ; Comparison designed to set Carry too
                JNE             $         ; Uh oh! ***STOP!***

                CPU 386                   ; From now on, we're confident!
