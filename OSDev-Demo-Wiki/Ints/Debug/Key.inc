;
; Ints/Debug/Key.inc
;

; This module handles the various keyboard actions.

Ints.Debug.Key:
                HLT                             ; Wait for interrupt

                MOV             AL,0            ; Consume any keypress
                XCHG            [FS:Data.Key.Code],AL ; Avoid repeats

                CMP             AL,0            ; Anything there?
                JE              .End            ; No, so leave
                CMP             AL,Debug.Key.Quit
                JE              .End            ; Quit?
                CMP             AL,Debug.Key.Format
                JE              .Format
                CMP             AL,Debug.Key.Up
                JE              .Up
                CMP             AL,Debug.Key.Down
                JE              .Down
                CMP             AL,Debug.Key.PgUp
                JE              .PgUp
                CMP             AL,Debug.Key.PgDn
                JE              .PgDn
                CMP             AL,Debug.Key.Prev
                JE              Ints.Debug.Desc.Prev
                CMP             AL,Debug.Key.Next
                JE              Ints.Debug.Desc.Next
.End:
                RET
;...............................................................................
Ints.Debug.Key.Format:
                CMP             EBP,Ints.Debug.Show.DWords
                JE              .Bytes
                MOV             EBP,Ints.Debug.Show.DWords
                JMP             .End
.Bytes:
                MOV             EBP,Ints.Debug.Show.Bytes
.End:
                RET
;...............................................................................
Ints.Debug.Key.Up:
                CMP             ESI,EBX          ; Reached Minimum?
                JBE             .End             ; Yep! Do nothing.

                SUB             ESI,Debug.BytesPerRow ; Move earlier in memory
.End:
                RET
;...............................................................................
Ints.Debug.Key.Down:
                MOV             EAX,ESI          ; Try some calculations
                ADD             EAX,Debug.Show.Height*Debug.BytesPerRow ; Add in bottom of Screen
                JO              .End             ; Overflow means too far!
                DEC             EAX              ; Need to, since Limit has -1
                CMP             EAX,EDX          ; Reached Maximum?
                JAE             .End             ; Yep, so too far

                ADD             ESI,Debug.BytesPerRow ; Move later in memory
.End:
                RET
;...............................................................................
Ints.Debug.Key.PgUp:
                MOV             CL,Debug.Show.Height-1 ; Leave one row the same
.Loop:
                CALL            Ints.Debug.Key.Up
                LOOP            .Loop
                RET
;...............................................................................
Ints.Debug.Key.PgDn:
                MOV             CL,Debug.Show.Height-1 ; Leave one row the same
.Loop:
                CALL            Ints.Debug.Key.Down
                LOOP            .Loop
                RET
;...............................................................................
Ints.Debug.Desc.Next:
                MOV             EAX,DS           ; Get current Descriptor
.Loop:
                ADD             AX,x86.Desc_size ; Go to next Descriptor
                CALL            Ints.Debug.Desc.Check
                JNE             .Loop
                RET

Ints.Debug.Desc.Prev:
                MOV             EAX,DS           ; Get current Descriptor
.Loop:
                SUB             AX,x86.Desc_size ; Go to previous Descriptor
                CALL            Ints.Debug.Desc.Check
                JNE             .Loop
                RET

Ints.Debug.Desc.Check:
; This checks the current Descriptor for readability. If not, it simply returns.
; If it is readable, it sets up the necessary registers; in particular the
; values for Minimum and Maximum.
; One step it does take is to check the result of the just-completed Next or
; Prev. If that over- or under-flowed, it swaps to the other Descriptor Table
; (GDT or LDT). Also note that since there's no easy way to determine the size
; of the current LDT, or even the GDT, it is easiest to simply check all the
; Descriptors.
                JNC             .Readable       ; Didn't overflow: keep going
                XOR             EAX,x86.Seg.TI  ; Use other Descriptor Table
.Readable:
                VERR            AX              ; Can I read it?
                JNZ             .End            ; Nope. Keep looking!

                MOV             DS,EAX          ; Found!

                LSL             EDX,EAX         ; Maximum address
                XOR             EBX,EBX         ; Minimum address
                MOV             ESI,EBX         ; Start at Minimum

                LAR             EAX,EAX         ; One more thing...
                TEST            AH,x86.Desc.Mem.Type.Code  ; Is it Code?
                JNZ             .Found                     ; OK, good
                TEST            AH,x86.Desc.Mem.Type.Stack ; No. Is it a Stack?
                JZ              .Found                     ; No! Phew!

                XCHG            EBX,EDX         ; Yes, so everything's upside down!
                INC             EBX             ; Or off-by-one
                NOT             EDX             ; Or inverted!
                ; Start at "top" of Segment, less a screenful
                MOV             ESI,0-(Debug.Show.Height*Debug.BytesPerRow) ; Screenful

                SHR             EAX,16          ; Also need to check Gran byte
                TEST            AL,x86.Desc.Mem.Gran.Big ; 32-bit?
                JNZ             .Found          ; Yes.
                MOV             AX,0FFFFh       ; No. Need to mask off high word
                AND             EDX,EAX         ; Maximum address
                AND             ESI,EAX         ; Starting address
.Found:
                XOR             EAX,EAX         ; Set Z flag - Found!
.End:
                RET
