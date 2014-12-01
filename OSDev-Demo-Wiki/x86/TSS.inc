;
; x86/TSS.inc
;

; This module defines the Task State Segment (the 32-bit one, anyway).

x86.TSS.Flags.Trap  EQU         0000_0000_0000_0001b

                STRUC           x86.TSS
.Back           RESW            1               ; Back-link to calling TSS
                RESW            1
.ESP0           RESD            1               ; Stack 0 pointer
.SS0            RESW            1
                RESW            1
.ESP1           RESD            1               ; Stack 1 pointer
.SS1            RESW            1
                RESW            1
.ESP2           RESD            1               ; Stack 2 pointer
.SS2            RESW            1
                RESW            1
.CR3            RESD            1               ; Page Directory Base Register
.EIP            RESD            1
.EFlags         RESD            1
.EAX            RESD            1
.ECX            RESD            1
.EDX            RESD            1
.EBX            RESD            1
.ESP            RESD            1
.EBP            RESD            1
.ESI            RESD            1
.EDI            RESD            1
.ES             RESW            1
                RESW            1
.CS             RESW            1
                RESW            1
.SS             RESW            1
                RESW            1
.DS             RESW            1
                RESW            1
.FS             RESW            1
                RESW            1
.GS             RESW            1
                RESW            1
.LDT            RESW            1
                RESW            1
.Flags          RESW            1
.IOMap          RESW            1
                ENDSTRUC
