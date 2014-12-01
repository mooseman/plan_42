;
; Ints/Fault.inc
;

; This is the general-purpose Fault handler. It simply writes out as much
; information as it can find onto the screen, to allow poor you to work out
; where the problem is!

Ints.Fault:
                PUSHAD                          ; Save all registers
                CALL            .Frame.Create   ; Create Stack Frame for display

                XOR             AX,AX           ; No TSS!
                MOV             FS,AX

                MOV             EDI,[ESP+64]    ; Load Interrupt number
                CALL            .ShowRegs

                CALL            .Frame.Destroy  ; Destroy created Stack Frame
                POPAD                           ; Note this doesn't replace ESP!
                ADD             ESP,8           ; Ignore Int and Error values
                IRETD                           ; And return
;...............................................................................
; This function pushes registers on the stack for display purposes - oh, and so
; I can use some of them! To standardise the stack layout and make the Ints.Names
; table consistent, other code may call it.
;
; Of course, the trick with using a function to save registers is that you call
; the function, then return from it, popping the values you just saved. But with
; assembly, you can perform a trick: POP the return address first, save what
; needs saving, PUSH the return address, and then RET. Actually, the last two
; steps can be done away with: just JMP to the saved register!

Ints.Fault.Frame.Create:
                POP             EDX             ; Pop off return address

                PUSH            DS              ; So these values can go there
                PUSH            ES
                PUSH            FS
                PUSH            GS
                PUSH            SS

                TEST    WORD    [ESP+64],x86.Seg.RPL3 ; From different Privilege Level?
                JZ              .SamePL         ; No

                MOV             EAX,[ESP+76]    ; Yes, so get old SS
                MOV             [ESP],EAX       ; Replace my value
                MOV             EAX,[ESP+72]    ; Get old ESP
                MOV             [ESP+32],EAX    ; Replace my value
.SamePL:
                STR             AX              ; Include faulting task
                PUSH            EAX
                SLDT            AX              ; And faulting LDT
                PUSH            EAX
                MOV             EAX,CR3         ; And Page Directory Base Register
                PUSH            EAX

                JMP             EDX             ; Return
;...............................................................................
Ints.Fault.Frame.Destroy:
                POP             EDX             ; Pop off return address

                ADD             ESP,16          ; Ignore saved CR3, LDT, TR and SS
                POP             GS              ; Pop everything else
                POP             FS
                POP             ES
                POP             DS

                JMP             EDX             ; Return
;...............................................................................
; This function is called externally (FAR), so that it can perform the NEAR call

Ints.Fault.External:
                CALL            Ints.Fault.ShowRegs
                RETF
;...............................................................................
Ints.Fault.ShowRegs:
                MOV             EBP,ESP         ; Save for display purposes

                CALL            Ints.System.Init
                PUSH            EAX

                PUSH            CS
                POP             DS              ; Point to code, for Ints.Names

                MOV             AX,Selector(GDT.VGA, GDT, RPL0)
                MOV             ES,AX           ; Point to screen with ES

                MOV             ESI,Ints.Names  ; Point to Names
                SHL             EDI,1           ; Turn IntNum into screen address
                INC     BYTE    [ES:EDI]        ; Dingle(tm) that this INT happened

                MOV             AH,Dev.VGA.RedBack | Dev.VGA.White ; White-on-red!
                ADD             EDI,VGA.Cols * 2 ; On next row

                CLD                             ; Work forwards
.Dump:
                MOV             ECX,Ints.Names.Label ; Width of label; zero high
                INC             ESI             ; Ignore Debug pos
.Name:
                LODSB                           ; Get character
                STOSW                           ; Store attrib+char
                LOOP            .Name           ; For each character
                MOV             AL,':'
                STOSW

                LODSB                           ; Get size of register
                MOV             CL,AL           ; Into counter

                MOV             EDX,FS          ; From TSS?
                TEST            EDX,EDX         ; Defined?
                JNZ             .InTSS          ; Yes. Go look at it
.OnStack:
                LODSB                           ; Get position on stack
                INC             ESI             ; Ignore position in TSS
                MOVZX           EDX,AL          ; Into memory offset
.FromStack:
                MOV             EDX,[EBP+EDX]   ; Get value from stack
                JMP             .Show
.InTSS:
                INC             ESI             ; Ignore Stack position
                LODSB                           ; Get position in TSS (FS)
                MOVZX           EDX,AL          ; Into memory offset
                CMP             AL,x86.TSS.CR3  ; Down low?
                JB              .FromStack      ; Yes. Must be on stack instead
                MOV             EDX,[FS:EDX]    ; No. So get value to show
.Show:
                PUSH            EDI             ; Save starting location
                CALL            Ints.Hex        ; Display as Hex
                POP             EDI
                ADD             EDI,(VGA.Cols-Ints.Names.Label-1)*2 ; Next row

                CMP             ESI,Ints.Names.End ; End of registers?
                JB              .Dump           ; No, so continue

                HLT                             ; Nothing more to do
; If interrupts are disabled, the HLT will "zombie" the PC, requiring a power
; off. VMware detects this condition, but it lets you look at the screen first!
; If interrupts are enabled, one will wake us up and continue from here, which
; will then return and resume the faulting instruction (presumably). Note that
; it may also invoke a fault-"storm", generating new faults that will overwrite
; what's just been written. In other words, what you see is the last thing that
; happened, not the first! To only see the first, uncomment the following line;
;               JMP             $-1
; Here's an example of what could happen: Say the User Code forgets to RET from
; a function, and that function is at the end of the Code Segment. The next
; instruction fetch will be past the end of CS, so a GPF will be raised. All of
; this .Fault code will display the problem (it might take you a bit to work out
; that CS has overflowed, but anyway...) and then this code will clean up and
; IRET to the problem area. But that's past CS, so now it is the .Fault's IRET
; that's the culprit! A new GPF will be generated, displaying the new details,
; blaming this very code!
                POP             EBX
                CALL            Ints.System.Done
                RET
