;
; User/Code.inc
;

; This module defines the User code to bounce the ball. It:
; 1) Uses the initial register values to set up the Data;
; 2) Draws the Frame (assuming the defined Window is big enough);
; 3) Calculates the bounce parameters (room to move vertically or horizontally?)
; 4) Bounces the ball around (or only increments it if the Window is too small);
; Forever!
;
; Note that this code can easily write outside its Window - by using direct
; Screen access, it can poke anything anywhere. It could have been given a
; 'strip' of the Screen to access, by defining an LDT-local alias to that part
; of the screen comprising the strip. That wouldn't help accesses to the left
; or right of the Window, but it would detect other bugs!
;
; On entry:
; CS:EIP = User.Entry
; SS:ESP = Stack
; ES     = Screen
; DS     = User.Data
; EBX    = Row pointer Delta
; CL     = Width
; CH     = Height
; DL     = Left-most position
; DH     = Top-most position

                SEGMENT         User  VSTART=0  ALIGN=16

User.ColouredBall EQU           (User.BallColour << 8) | User.Ball

User.Entry:
                CLD                             ; Work forwards
                MOV             [User.Data.Row],EBX
                MOV             [User.Data.Left],DX  ; Left and Top
                MOV             [User.Data.Width],CX ; Width and Height

                PUSH            ES
                POP             DS              ; Don't need DS anymore

                MOV             EBP,EBX         ; Save this away

                ; Calculate Screen pointer
                MOV             AL,DH           ; Get Top
                MUL             BL              ; Multiply by row offset
                MOVZX           EDI,AX          ; Put into Screen ptr

                MOVZX           EAX,DL          ; Get Left
                LEA             EDI,[EDI+EAX*2] ; Add Left into Screen ptr

                ; Check if Frame is needed
                CMP             CL,2            ; Wide enough for frame?
                JBE             .DeltaX         ; Nope!
                CMP             CH,2            ; Tall enough for frame?
                JBE             .DeltaX         ; Nope!

                ; Reduce dimensions, Draw Frame, and move inside
                MOV             ESI,EDI         ; Remember current screen ptr
                SUB             CX,0202h        ; Shrink both width and height
                CALL            User.Frame      ; Draw frame...
                LEA             EDI,[ESI+EBP+2] ; ...and point inside it

.DeltaX:        ; Calculate DeltaX
                MOV             BL,+1           ; Set DeltaX
                MOV             ESI,2           ; Use this for DeltaX on pointer
                CMP             CL,1            ; Too narrow?
                JA              .DeltaY         ; Nope!
                MOV             BL,0            ; No DeltaX
                XOR             ESI,ESI         ; No DeltaX pointer

.DeltaY:        ; Calculate DeltaY
                MOV             BH,+1           ; Set DeltaY
;               MOV             EBP,EBP         ; Use this for DeltaY on pointer
                CMP             CH,1            ; Too short?
                JA              .Start          ; Nope!
                MOV             BH,0            ; No DeltaY
                XOR             EBP,EBP         ; No DeltaY pointer

.Start:
; Now that all the maths is out of the way, I need the pos to be 1-relative (not
; 0-relative)
                MOV             DH,1            ; Start Y
                MOV             DL,1            ; Start X
.Move:
                MOV             EAX,EDI         ; Remember old pointer

.MoveH:
                ADD             EDI,ESI         ; Add in DeltaX pointer
                ADD             DL,BL           ; Add in DeltaX
                JZ              .BounceH        ; Underflow?
                CMP             DL,CL           ; No. Overflow?
                JBE             .MoveV          ; No. No bounce!
.BounceH:
                NEG             ESI             ; Reverse DeltaX pointer
                NEG             BL              ; Reverse DeltaX
                LEA             EDI,[EDI+ESI*2] ; Fix up pointer
                ADD             DL,BL           ; Fix up position
                ADD             DL,BL           ; And move correctly

.MoveV:
                ADD             EDI,EBP         ; Add in DeltaY pointer
                ADD             DH,BH           ; Add in DeltaY
                JZ              .BounceV        ; Underflow?
                CMP             DH,CH           ; No. Overflow?
                JBE             .Update         ; No. No bounce!
.BounceV:
                NEG             EBP             ; Reverse DeltaY pointer
                NEG             BH              ; Reverse DeltaY
                LEA             EDI,[EDI+EBP*2] ; Fix up pointer
                ADD             DH,BH           ; Fix up position
                ADD             DH,BH           ; And move correctly

.Update:
                CMP             EDI,EAX         ; After all that, did we move?
                JE              .Increment      ; Nope! Just increment.
                MOV     BYTE    [EAX],' '       ; Erase old ball
                MOV     WORD    [EDI],User.ColouredBall ; And write in new one
                JMP             .Wait
.Increment:
                INC     BYTE    [EDI]           ; Increment screen position

.Wait:
;               HLT                             ; Illegal User-mode instruction!

                User.Halt                       ; So, use system-provided one

;               User.Yield                      ; This has a different effect

; This is an alternative to using HLT
;                PUSH            ECX
;                MOV             ECX,User.Delay  ; Wait for a little while
;                LOOP            $               ; About this long...
;                POP             ECX

                JMP             .Move
