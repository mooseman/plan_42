;
; Exec/Ints/Fault.inc
;

; This module is the generic Fault handler, for Faults that need their own
; runtime environment: Stack, TSS, etc. It presumes the following:
; 1) The LDT has three consecutive entries for the Fault Handler:
;    a) Stack - This will be created
;    b) TSS   - This will be created and set up.
;    c) Alias - This will be used by the Fault handler to alias the Faulting TSS
; 2) The .Fault function installs the Handler using the provided values.
; 3) The .Handler function will be called by the specific Fault handler.

Exec.Ints.Fault:

Exec.Ints.Fault.Stack.Size   EQU   1000h        ; Give ourselves PLENTY of room!

%push Ints.Fault ; Let's not leave these %defines just lying around...

%define         %$IDT.Entry     EBP - 4
%define         %$LDT.Entry     EBP - 8
%define         %$Handler       EBP - 12
%define         %$Base          EBP - 16
%define         %$TSS           EBP - 20

                ENTER           20, 0

                MOV             [%$IDT.Entry],EAX
                MOV             [%$LDT.Entry],EBX
                MOV             [%$Handler],EDX

                MOV             AX,Selector(Exec.LDT.Alias, LDT, RPL0)
                MOV             ES,AX         ; Point to LDT alias

                ; Allocate Stack for Fault handler
                MOV             ECX,Exec.Ints.Fault.Stack.Size
                CALL            Exec.Alloc.RAM
                TEST            EAX,EAX
                JZ              .End

                ; Store in LDT
                MOV             DL,Type.Mem(Stack, DPL0, RW)
                MOV             DH,Gran.Mem(Byte,  Big)
                CALL            Exec.Alloc.DT.Mem

                ; Allocate TSS
                SLDT            AX            ; My LDT
                CALL            Exec.Alloc.TSS
                TEST            EAX,EAX
                JZ              .End

                ; Remember its Base
                MOV             [%$Base],EAX

                MOV             EBX,[%$LDT.Entry]
                OR              BX,WORD x86.Seg.TI      ; Index inside LDT
                MOV             EDX,[%$Handler]

                MOV             [ES:x86.TSS.SS],BX
                MOV             [ES:x86.TSS.CS],CS
                MOV             [ES:x86.TSS.EIP],EDX

                MOV             AH,System.TSS   ; Oh BOY is it a System TSS!
                CALL            Exec.Alloc.TSS.Enable

                MOV             [%$TSS],EBX

                MOV             AX,Selector(Exec.LDT.Alias, LDT, RPL0)
                MOV             ES,AX             ; Point to LDT alias again

                MOV             EAX,[%$Base]
                MOV             EBX,[%$LDT.Entry]
                ADD             EBX,x86.Desc_size ; Point to .TSS alias in LDT
                MOV             DL,Type.Mem(Data, DPL0, NoRW) ; TSS Alias
                MOV             DH,Gran.Mem(Byte, Small)
                CALL            Exec.Alloc.DT.Mem

                ; Finally, update IDT Entry to tell it it's now handled by a TSS
                XOR             EAX,EAX
                MOV             EBX,[%$IDT.Entry]
                MOV             ECX,[%$TSS]
                MOV             DL,Type.Sys(Task, DPL0, 286) ; No '386 variant
                CALL            Exec.Alloc.IDT
.End:
                LEAVE
                RET
%pop
;...............................................................................
.Handler:
                STI                           ; Interrupts may have been off

.TestIRQs:
                ; Worse, Fault may have happened during an IRQ handler!
                ; Isolate IRQ handlers that have started but aren't acknowledged
                MOV             AL,Dev.PIC.Cmd.ISR
                OUT             Dev.PIC.A.Cmd,AL
                JMP             $+2             ; Wait for I/O bus
                IN              AL,Dev.PIC.A.Cmd

                AND             AL,Dev.PIC.A.Key|Dev.PIC.A.Timer; Only worried about these
                JZ              .NoInterrupts    ; Nothing to worry about!
                CMP             AL,Dev.PIC.A.Key ; If keypress outstanding...
                JE              .KeyInterrupt    ; Handle it
                MOV             AL,Dev.PIC.Cmd.EOI ; Acknowledge
                OUT             Dev.PIC.A.Cmd,AL ; current IRQ (should be Timer)
                JMP             .TestIRQs        ; And keep looking

.KeyInterrupt:
                INT             (IDT.Key-IDT)/x86.Desc_size ; Re-invoke the Key interrupt
; Continue
.NoInterrupts:
                MOV             AX,Selector(GDT.Alias, GDT, RPL0)
                MOV             DS,AX         ; Need information from GDT
                MOV             AX,Selector(Exec.LDT.Alias, LDT, RPL0)
                MOV             ES,AX         ; Need to update alias in LDT
                LEA             AX,[BX + x86.Seg.TI] ; Need my TSS alias in LDT
                MOV             FS,AX

                ADD             EBX,x86.Desc_size ; Point to TSS alias in LDT

                MOVZX           ESI,WORD [FS:x86.TSS.Back] ; Who called me?

                PUSH            ESI               ; Need this on Stack to display Task

                ; Copy faulting TSS definition, then alter .Type to make it an alias
                CLD                               ; Work forwards
                MOV             ECX,x86.Desc_size / 4      ; Number of DWords
                MOV             EDI,EBX
                REP             MOVSD             ; Brute force details across!
                MOV             [ES:EBX+x86.Desc.Type],BYTE Type.Mem(Data, DPL0, NoRW)

                LEA             AX,[BX + x86.Seg.TI] ; TSS alias in LDT
                MOV             FS,AX

                MOV             EDI,[ESP+8]       ; Load Interrupt number
                CALL            GDT.Ints : Ints.Fault.External

                ADD             ESP,4             ; Ignore Back Task
                RET
