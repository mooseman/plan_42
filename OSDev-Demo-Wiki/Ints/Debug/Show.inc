;
; Ints/Debug/Show.inc
;

; This module has the various Display functions to show the current Segment.

;-------------------------------------------------------------------------------
; This function shows a screenful of memory, without violating any Segment
; limits. It displays the memory in either Byte (with ASCII) or DWord form.
; Input:  DS  = Segment to display
;         ES  = Screen Segment
;         EBX = Minimum displayable address (0000_0000h or Limit)
;         EDX = Maximum displayable address (Limit or FFFF_FFFFh)
;         ESI = Current address to display from
;         EBP = .Byte or .DWord function pointer
; Output: EAX, ECX, EDI modified

Ints.Debug.Show:
                PUSH            ESI

                MOV             ECX,Debug.Show.Height
                MOV             EDI,(Debug.VGA.Top*VGA.Cols+Debug.VGA.Left)*2
.Row:
                PUSH            ECX               ; Need this later

                CMP             ESI,EDX           ; Past Maximum?
                JA              .Row.Blank        ; Yes, so blank row
                CMP             ESI,EBX           ; At least Minimum?
                JAE             .Row.Show         ; Yes, so show row normally
                LEA             EAX,[EBX-Debug.BytesPerRow] ; Is it a partial row?
                CMP             ESI,EAX           ; Below even this?
                JA              .Row.Show         ; No, so show partial row
.Row.Blank:
                MOV             AH,Debug.Colour.Blank
                MOV             AL,' '            ; Blank
                MOV             ECX,Debug.VGA.Width   ; Entire row
                ADD             ESI,Debug.BytesPerRow ; Catching up?
                REP             STOSW
                JMP             .Row.Next         ; Next row
.Row.Show:
                PUSH            EDX               ; Need this later
                CALL            .Address
                POP             EDX

                CALL            EBP               ; Call current Row function

.Row.Next:
                ADD             EDI,(VGA.Cols-Debug.VGA.Width)*2
                POP             ECX               ; More rows?
                LOOP            .Row

                POP             ESI               ; Back to top
                RET
;...............................................................................
.Address:
                MOV             AH,Debug.Colour.Addr

                MOV             CL,4              ; Size of Selector
                MOV             DX,DS             ; Selector to display
                CALL            Ints.Hex

                MOV             AL,':'            ; Separator
                STOSW

                MOV             CL,8
                MOV             EDX,ESI           ; Address
                CALL            Ints.Hex

                MOV             AL,' '
                STOSW
                STOSW
                STOSW

                RET
;...............................................................................
; This function displays one row of .BytesPerRow Bytes (if inside Limits), plus
; an ASCII representation on the side.

Ints.Debug.Show.Bytes:
                MOV             AH,Debug.Colour.Bytes
                MOV             CL,Debug.BytesPerRow ; Number of Bytes
.Loop:
                PUSH            ECX

                MOV             AH,Debug.Colour.ASCII
                LEA             ECX,[Debug.BytesPerRow+ECX*2] ; Offset to ASCII field

                CMP             ESI,EBX           ; Less than Minimum?
                JB              .NoByte           ; Yes, so skip
                CMP             ESI,EDX           ; More than Maximum?
                JA              .NoByte           ; Yes, so skip

                LODSB                             ; Byte to display
                MOV             [ES:EDI+ECX*2],AX ; Store ASCII

                PUSH            EDX
                MOV             AH,Debug.Colour.Bytes
                MOV             CL,2              ; Number of nybbles
                MOV             DL,AL
                CALL            Ints.Hex
                POP             EDX

                MOV             AL,' '
                CMP     BYTE   [ESP],4            ; Don't show in last position
                JB              .Separator
                TEST            ESI,0003h
                JNZ             .Separator
                MOV             AL,'-'
.Separator:
                STOSW
                JMP             .Next
.NoByte:
                MOV             AL,' '
                INC             ESI               ; Catching up?
                MOV             [ES:EDI+ECX*2],AX ; Store ASCII

                MOV             AH,Debug.Colour.Bytes
                MOV             ECX,3
                REP             STOSW
.Next:
                POP             ECX
                LOOP            .Loop

                ; Now blank out any area to the right of the ASCII field
                MOV             AL,' '
                MOV             AH,Debug.Colour.ASCII
                MOV             ECX,Debug.VGA.Width-Debug.Width.Addr-Debug.Width.Byte
                ADD             EDI,Debug.BytesPerRow*2 ; Go past ASCII field
                REP             STOSW
                RET
;...............................................................................
; This function displays one row of 4 DWords (if inside Limits). There's some
; hairy code about trying to display part-words at the edge of the Limits...

Ints.Debug.Show.DWords:
                MOV             CL,Debug.BytesPerRow/4 ; Number of DWords
.Loop:
                PUSH            ECX

                CMP             ESI,EDX           ; More than Maximum?
                JA              .NoDWord          ; Yes, so skip
                CMP             ESI,EBX           ; Less than Minimum?
                JAE             .TestHi           ; No, so TestHi
                LEA             EAX,[EBX-4]       ; Check how FAR less we are
                CMP             ESI,EAX           ; MUCH less?
                JBE             .NoDWord          ; Yes
                JMP             .Partial          ; Get part-DWord
.TestHi:
                LEA             EAX,[EDX-3]       ; Test how close to Maximum
                CMP             ESI,EAX
                JBE             .Normal           ; Safe to proceed normally
.Partial:
                XOR             EAX,EAX           ; Start with zero
                MOV             CL,4              ; Number of bytes
.Build:
                SHL             EAX,8             ; One more byte
                CMP             ESI,EBX           ; Still too low?
                JB              .Continue         ; Yep!
                CMP             ESI,EDX           ; Now too high?
                JA              .Continue         ; Yep!
                MOV             AL,[ESI]          ; Nope, so safe!
.Continue:
                INC             ESI               ; Keep looking
                LOOP            .Build            ; Build DWord
                JMP             .Show
.Normal:
                LODSD                             ; DWord to display
.Show:
                PUSH            EDX
                MOV             CL,8              ; Number of nybbles
                MOV             EDX,EAX
                MOV             AH,Debug.Colour.DWords
                CALL            Ints.Hex
                POP             EDX
                JMP             .Separator
.NoDWord:
                MOV             AH,Debug.Colour.DWords
                MOV             AL,' '
                MOV             ECX,8
                ADD             ESI,4             ; Catching up?
                REP             STOSW
.Separator:
                MOV             AL,' '
                STOSW
                STOSW
.Next:
                POP             ECX
                LOOP            .Loop

                ; Now blank out any area to the right of the Hex field
                MOV             AL,' '
                MOV             ECX,Debug.VGA.Width-Debug.Width.Addr-Debug.Width.DWord
                REP             STOSW
                RET
