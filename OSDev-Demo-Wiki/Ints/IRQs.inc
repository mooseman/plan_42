;
; Ints/IRQs.inc
;

; This file holds the entry points for all of the Interrupt ReQuests (IRQs).
; They all jump to the same handler, but not before saving away the number
; of the interrupt - oh, and a fault code since these should not happen!

; To save space, the handler has been put in the middle of the vectors, to
; allow the assembler to use shorter JMPs.
Ints.IRQ0:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    32      ; Interrupt number
                JMP             Ints.PIC.A
Ints.IRQ1:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    33      ; Interrupt number
                JMP             Ints.PIC.A
Ints.IRQ2:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    34      ; Interrupt number
                JMP             Ints.PIC.A
Ints.IRQ3:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    35      ; Interrupt number
                JMP             Ints.PIC.A
Ints.IRQ4:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    36      ; Interrupt number
                JMP             Ints.PIC.A
Ints.IRQ5:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    37      ; Interrupt number
                JMP             Ints.PIC.A
Ints.IRQ6:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    38      ; Interrupt number
                JMP             Ints.PIC.A
Ints.IRQ7:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    39      ; Interrupt number
                JMP             Ints.PIC.A

; Acknowledge Slave PIC (first)
Ints.PIC.B:
                PUSH            EAX
                MOV             AL,Dev.PIC.Cmd.EOI
                OUT             Dev.PIC.B.Cmd,AL
                POP             EAX
; Acknowledge Master PIC
Ints.PIC.A:
                PUSH            EAX
                MOV             AL,Dev.PIC.Cmd.EOI
                OUT             Dev.PIC.A.Cmd,AL
                POP             EAX
; All IRQs jump to the default Fault handler
                JMP             Ints.Fault

Ints.IRQ8:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    40      ; Interrupt number
                JMP             Ints.PIC.B
Ints.IRQ9:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    41      ; Interrupt number
                JMP             Ints.PIC.B
Ints.IRQ10:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    42      ; Interrupt number
                JMP             Ints.PIC.B
Ints.IRQ11:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    43      ; Interrupt number
                JMP             Ints.PIC.B
Ints.IRQ12:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    44      ; Interrupt number
                JMP             Ints.PIC.B
Ints.IRQ13:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    45      ; Interrupt number
                JMP             Ints.PIC.B
Ints.IRQ14:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    46      ; Interrupt number
                JMP             Ints.PIC.B
Ints.IRQ15:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    47      ; Interrupt number
                JMP             Ints.PIC.B
