;
; Ints/Single.inc
;

; This module provides an example Single-Step handler interrupt.
; To invoke it, enable Single Step Interrupt with the "TRACE" macro.
; It waits for a keypress:
; If it's a Space, it simply returns - the next instruction will stop again.
; If it's Enter, it returns too - but first, it disables the Trace Flag to
; stop future interrupts.
;
; A good place to put the "TRACE" macro is inside the User code, just as the
; Frame is being drawn. Then, EVERY Task will be in Single Step mode! To
; continue the Tasks, merely hold down the Space key, and watch them slowly
; build their Frames, and perhaps start to bounce their balls.
;
; If you press Enter, one of the Tasks will resume as normal. If you hold down
; Enter, more and more of the Tasks will resume...

Ints.Single:

.Key.Continue   EQU             Dev.Key.Space
.Key.Stop       EQU             Dev.Key.Enter

                PUSH            EAX             ; Need these registers
                PUSH            DS
                PUSH            ES

                MOV             AX,Selector(GDT.Data, GDT, RPL0) ; Point to Data
                MOV             DS,AX

                MOV             AX,Selector(GDT.VGA,  GDT, RPL0) ; Point to Screen
                MOV             ES,AX
.Loop:
                INC     BYTE    [ES:0002h]       ; Dingle(tm) screen location

                HLT                              ; Wait for interrupt

                MOV             AL,0             ; Consume any keypress
                XCHG            [Data.Key.Code],AL
                CMP             AL,.Key.Continue ; Continue?
                JE              .Finish          ; Yes, so leave
                CMP             AL,.Key.Stop     ; Stop?
                JNE             .Loop            ; No, so keep waiting

                AND     WORD    [ESP+14h],~x86.EFlags.TF; Yes, so stop tracing
.Finish:
                POP             ES
                POP             DS
                POP             EAX
                IRETD
