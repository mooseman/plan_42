;
; Boot/PM.inc
;

; This code does the final preparations before launching into Protected Mode.
; It loads GDTR, IDTR, switches to Protected Mode, loads LDTR then JMPs to the
; Executive.

Boot.LDT.Code   EQU             Selector(Exec.LDT.Code, LDT, RPL0)

Boot.PM:
                MOV             AX,GDT.Base >> 4 ; Point to GDT
                MOV             DS,AX
                LGDT            [GDT.Alloc.Pseudo]

                PUSH   DWORD    IDT.Base    ; Put on stack as Pseudo-descriptor
                PUSH    WORD    IDT.Limit   ; Only this many entries in IDT
                LIDT            [ESP]
; We're about to abandon the stack, so don't bother fixing it up!

; Can only load the LDTR after we've entered Protected Mode: this is the value
                MOV             BX,Selector(GDT.Exec.LDT, GDT, RPL0)

; Turn on Protected Mode (cross your fingers!)
                CLI                             ; Disable interrupts!
                MOV             EAX,CR0
                OR              AL,x86.CR0.PE
                MOV             CR0,EAX

; First, load the LDT
                LLDT            BX
; Then, launch code
                JMP             Boot.LDT.Code : DWORD Exec.Code

; This code is now unneeded. It can be overwritten by whatever the system wants
