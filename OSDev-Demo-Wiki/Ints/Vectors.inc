;
; Ints/Vectors.inc
;

; This file holds the entry points for all of the Intel-reserved interrupts.
; They all jump to the same handler, but not before saving away the number
; of the interrupt - oh, and a pseudo-fault code if one isn't already generated.

; To save space, the handler has been put in the middle of the vectors, to
; allow the assembler to use th shorter form of the JMP instruction.

                SEGMENT    Ints  VSTART=0  ALIGN=16

                USE32

Ints.Int0:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    0       ; Interrupt number
                JMP             Ints.Vector
Ints.Int1:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    1       ; Interrupt number
                JMP             Ints.Vector
Ints.Int2:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    2       ; Interrupt number
                JMP             Ints.Vector
Ints.Int3:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    3       ; Interrupt number
                JMP             Ints.Vector
Ints.Int4:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    4       ; Interrupt number
                JMP             Ints.Vector
Ints.Int5:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    5       ; Interrupt number
                JMP             Ints.Vector
Ints.Int6:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    6       ; Interrupt number
                JMP             Ints.Vector
Ints.Int7:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    7       ; Interrupt number
                JMP             Ints.Vector
Ints.Int8:
;               PUSH   DWORD    0       ; Pseudo-fault code (not required)
                PUSH   DWORD    8       ; Interrupt number
                JMP             Ints.Vector
Ints.Int9:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    9       ; Interrupt number
                JMP             Ints.Vector
Ints.Int10:
;               PUSH   DWORD    0       ; Pseudo-fault code (not required)
                PUSH   DWORD    10      ; Interrupt number
                JMP             Ints.Vector
Ints.Int11:
;               PUSH   DWORD    0       ; Pseudo-fault code (not required)
                PUSH   DWORD    11      ; Interrupt number
                JMP             Ints.Vector
Ints.Int12:
;               PUSH   DWORD    0       ; Pseudo-fault code (not required)
                PUSH   DWORD    12      ; Interrupt number
                JMP             Ints.Vector
Ints.Int13:
;               PUSH   DWORD    0       ; Pseudo-fault code (not required)
                PUSH   DWORD    13      ; Interrupt number
                JMP             Ints.Vector
Ints.Int14:
;               PUSH   DWORD    0       ; Pseudo-fault code (not required)
                PUSH   DWORD    14      ; Interrupt number
                JMP             Ints.Vector
Ints.Int15:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    15      ; Interrupt number
;               JMP             Ints.Vector

Ints.Vector     JMP             Ints.Fault

Ints.Int16:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    16      ; Interrupt number
                JMP             Ints.Vector
Ints.Int17:
;               PUSH   DWORD    0       ; Pseudo-fault code (not required)
                PUSH   DWORD    17      ; Interrupt number
                JMP             Ints.Vector
Ints.Int18:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    18      ; Interrupt number
                JMP             Ints.Vector
Ints.Int19:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    19      ; Interrupt number
                JMP             Ints.Vector
Ints.Int20:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    20      ; Interrupt number
                JMP             Ints.Vector
Ints.Int21:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    21      ; Interrupt number
                JMP             Ints.Vector
Ints.Int22:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    22      ; Interrupt number
                JMP             Ints.Vector
Ints.Int23:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    23      ; Interrupt number
                JMP             Ints.Vector
Ints.Int24:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    24      ; Interrupt number
                JMP             Ints.Vector
Ints.Int25:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    25      ; Interrupt number
                JMP             Ints.Vector
Ints.Int26:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    26      ; Interrupt number
                JMP             Ints.Vector
Ints.Int27:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    27      ; Interrupt number
                JMP             Ints.Vector
Ints.Int28:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    28      ; Interrupt number
                JMP             Ints.Vector
Ints.Int29:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    29      ; Interrupt number
                JMP             Ints.Vector
Ints.Int30:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    30      ; Interrupt number
                JMP             Ints.Vector
Ints.Int31:
                PUSH   DWORD    0       ; Pseudo-fault code
                PUSH   DWORD    31      ; Interrupt number
                JMP             Ints.Vector
