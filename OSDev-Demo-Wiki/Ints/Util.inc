;
; Ints/Util.inc
;

; This module holds Utility functions for various Fault handlers to use

;...............................................................................
; This function switches to the next available Task. Note that it assumes that
; interrupts are OFF!
;
; It cycles through the Global Descriptors looking for Idle TSSs. When it finds
; one, it JMPs to it - stopping the current Task, making it Idle for next time.
;
; Note that I use x86.Desc.Mem.Gran.Avail to flag a System TSS: it may be Idle
; now, but that'd be because nothing's using it yet -  a Fault handler is a
; good example of a System TSS waiting to be used.
;
; Also note that if the TSS that is currently running is a System TSS, I won't
; switch away from it either!

Ints.Switch:

.Type.Mask      EQU             x86.Desc.Type.Present | x86.Desc.Type.Sys | x86.Desc.Type.Type
.IdleTSS        EQU             Type.Sys(TSS, DPL0, 386) ; Target!

                STR             EAX             ; Get current TSS
                TEST            EAX,EAX         ; Switching Tasks yet?
                JZ              .End            ; No, so leave

                PUSH            EBX             ; Need some more registers
                PUSH            DS              ; And a Segment register

                MOV             EBX,EAX
                MOV             AX,Selector(GDT.Alias, GDT, RPL0)
                MOV             DS,AX           ; To point to the GDT

                ; I know that EAX holds a TSS - but is it a System TSS?
                TEST    BYTE    [EBX+x86.Desc.Mem.Gran],System.TSS
                JNZ             .Kill           ; Yes! Abort!

                PUSH            EDX             ; Now need this

                MOV             EDX,EBX         ; Starting point
.Loop:
                ADD             BX,x86.Desc_size     ; Look at next descriptor
                JZ              .Restart             ; Overflow!
                CMP             BX,[GDT.Alloc.Limit] ; Too far?
                JB              .Continue            ; Not yet...
.Restart:
                MOV             BX,GDT.Limit+1       ; Yes. Start at original Limit
.Continue:
                CMP             EBX,EDX         ; Looped back to here again?
                JE              .Finish         ; Yes, so nothing to switch to

                MOV             AL,[EBX+x86.Desc.Mem.Type] ; Get Descriptor Type
                AND             AL,.Type.Mask   ; Isolate interesting bits
                CMP             AL,.IdleTSS     ; Is it an Idle TSS?
                JNE             .Loop           ; No. Keep looking

                TEST    BYTE    [EBX+x86.Desc.Mem.Gran],System.TSS
                JNZ             .Loop           ; Ignore System TSSs too

                PUSH            EBX             ; Found! So JMP to it!
                PUSH            0               ; (Note that Offset is ignored)
                JMP     FAR     [ESP]           ; Invokes Task Switch
                ADD             ESP,8           ; When JMPs back, continues here
.Finish:
                POP             EDX
.Kill:
                POP             DS
                POP             EBX
.End:
                RET
;...............................................................................
; This function sets the current TSS to be a System TSS.
; Output: EAX = whether it modified the Task. Pass this in EBX to .Done
;         GS  = GDT
Ints.System.Init:
                MOV             AX,Selector(GDT.Alias, GDT, RPL0)
                MOV             GS,AX           ; Point to GDT

                STR             EAX             ; Get current Task
                TEST            EAX,EAX         ; Is there one?
                JZ              .End            ; No, so nothing to do
                TEST    BYTE    [GS:EAX+x86.Desc.Mem.Gran],System.TSS
                JNZ             .System         ; Already a System TSS!
                OR      BYTE    [GS:EAX+x86.Desc.Mem.Gran],System.TSS
                JMP             .End            ; Make it one temporarily
.System:
                XOR             EAX,EAX         ; Already a System TSS! Don't touch!
.End:
                RET
;...............................................................................
; This function reverts any change made by .Init (above)
; Input:  EBX = EAX output from .Init
;         GS  = GDT

Ints.System.Done:
                TEST            EBX,EBX         ; Was current TSS modified?
                JZ              .End            ; No, so just leave
                AND     BYTE    [GS:EBX+x86.Desc.Mem.Gran],~System.TSS
.End:
                RET
;...............................................................................
; This function displays a Hex value on the screen.
; Input:  AH  = Colour to use
;         ECX = Number of digits to display
;         EDX = Value to display
;         EDI = Screen position to display
;         ES  = Screen segment
; Output: AL, EDX modified. ECX zeroed. EDI preserved
Ints.Hex:
                STD                             ; Work backwards
                LEA             EDI,[EDI+ECX*2-2] ; Point to end of number
                PUSH            EDI             ; Save for end
.Loop:
                MOV             AL,DL           ; Get lowest byte
                AND             AL,0Fh          ; Isolate low nybble
                ADD             AL,'0'          ; Turn into ASCII
                CMP             AL,'9'          ; Too far?
                JBE             .NotHex         ; No
                ADD             AL,'A'-'9'-1    ; Yes, so turn into Hex
.NotHex:
                STOSW                           ; Store
                SHR             EDX,4           ; Shift in next nybble
                LOOP            .Loop           ; Loop
                CLD                             ; Assume forwards
                POP             EDI             ; Saved end
                ADD             EDI,2           ; Past last digit
                RET                             ; And return
;-------------------------------------------------------------------------------
; The following table lists the information for each register to show:
; Debug offset, Label, Num Hex chars, Stack offset, TSS offset

Ints.Names:
.Label          EQU             4  ; Width of label
;                              Debug,  Name , W,Stk,TSS
                DB                 0, ' Int', 2, 68, 20 ; On stack
                DB                 0, ' Err', 4, 72, 24 ; On stack
.Regs: ; Only registers from here
                DB              002h, ' EAX', 8, 64, 40
                DB              011h, ' EBX', 8, 52, 52
                DB              020h, ' ECX', 8, 60, 44
                DB              02Fh, ' EDX', 8, 56, 48
                DB              042h, ' ESI', 8, 40, 64
                DB              051h, ' EDI', 8, 36, 68
                DB              060h, ' EBP', 8, 44, 60
                DB              0DAh, ' ESP', 8, 48, 56
                DB              0C7h, ' EIP', 8, 76, 32
                DB              0C2h, '  CS', 4, 80, 76
                DB              082h, '  DS', 4, 32, 84
                DB              08Dh, '  ES', 4, 28, 72
                DB              098h, '  FS', 4, 24, 88
                DB              0A3h, '  GS', 4, 20, 92
                DB              0D5h, '  SS', 4, 16, 80
                DB              06Fh, 'Flag', 8, 84, 36
                DB              0AFh, 'PDBR', 8,  4, 28 ; Minimum NOT on stack
                DB              0F3h, 'Task', 4, 12, 12 ; On stack
                DB              0E9h, ' LDT', 4,  8, 96
.End            EQU             $
;    0000000000000000111111111111111122222222222222223333333333333333
;    0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF
;000:  _EAX:########  _EBX:########  _ECX:########  _EDX:########
;040:  _ESI:########  _EDI:########  _EBP:########  Flag:########
;080:  __DS:####  __ES:####  __FS:####  __GS:####   PDBR:########
;0C0:  __CS:####:######## __SS:####:######### _LDT:#### Task:####
