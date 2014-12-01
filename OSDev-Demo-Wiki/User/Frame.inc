;
; User/Frame.inc
;

; This function draws the frame using ASCII art and OEM characters
User.Frame:
                PUSH            ECX

                MOV             AH,User.FrameColour
                MOV             EBX,ECX         ; Save this away

                XOR             ECX,ECX         ; Zero high part of ECX

                MOV             EDX,EDI         ; Save this away
                MOV             AL,'Ú'          ; TopLeft corner
                STOSW                           ; Store on screen
                MOV             AL,'Ä'          ; Top
                MOV             CL,BL           ; This many
                REP             STOSW           ; Store on screen
                MOV             AL,'¿'          ; TopRight corner
                STOSW                           ; Store on screen

                LEA             EDI,[EDX+EBP]   ; Go to start of next row
                MOV             CL,BH           ; Height
                MOVZX           EBX,BL          ; Don't need BH any more
.Side:
                MOV             EDX,EDI         ; Save this away
                MOV             AL,'³'          ; Side
                STOSW                           ; Store on screen
                LEA             EDI,[EDI+EBX*2] ; Move to right side
                STOSW                           ; Store on screen

                LEA             EDI,[EDX+EBP]   ; Go to start of next row
                LOOP            .Side

;               MOV             EDX,EDI         ; Save this away
                MOV             AL,'À'          ; Bottom left
                STOSW                           ; Store on screen
                MOV             AL,'Ä'          ; Bottom
                MOV             CL,BL           ; This many
                REP             STOSW           ; Store on screen
                MOV             AL,'Ù'          ; Bottom right
                STOSW                           ; Store on screen
;               LEA             EDI,[EDX+EBP]   ; (Last row anyway)

                POP             ECX
                RET
