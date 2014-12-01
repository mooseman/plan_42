;
; Exec/Timer.inc
;

; This module installs a Timer interrupt handler, then sets the Programmable
; Interval Timer (PIT) to interrupt often. The Interrupt handler will cycle
; through the active Tasks, emulating simultaneity.

Exec.Timer:
; First, install a Timer interrupt handler
                MOV             EAX,Ints.Timer  ; Offset
                MOV             EBX,IDT.Timer   ; Interrupt
                MOV             CX,Selector(GDT.Ints, GDT, RPL0)
                MOV             DL,Type.Sys(Int, DPL0, 386)
                CALL            Exec.Alloc.IDT

; Then, reprogram the Programmable Interrupt Timer to the desired frequency
                MOV             AX,Dev.Timer.ClockFreq / Timer.Tick

                OUT             Dev.Timer.Port,AL ; Low byte
                JMP             $+2               ; (Wait for it...)
                MOV             AL,AH             ; High byte
                OUT             Dev.Timer.Port,AL

; Finally, enable the Timer interrupt in the PIC
                IN              AL,Dev.PIC.A.Mask
                AND             AL,~Dev.PIC.A.Timer ; Unmask Timer IRQ
                OUT             Dev.PIC.A.Mask,AL

                RET
