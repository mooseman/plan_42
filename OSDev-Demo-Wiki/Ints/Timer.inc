;
; Ints/Timer.inc
;

; This module implements the Timer interrupt handler.
; It simply performs some housekeeping and then calls the generic Task
; Switching code.

Ints.Timer:
                PUSH            EAX             ; Need this register

                MOV             AL,Dev.PIC.Cmd.EOI ; Acknowledge interrupt in PIC
                OUT             Dev.PIC.A.Cmd,AL   ; (Note interrupts still off)

                CALL            Ints.Switch     ; Switch Task

                POP             EAX
                IRETD                           ; And return where we left off
