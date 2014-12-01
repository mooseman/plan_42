;
; Ints/Debug/Regs.inc
;

; This module displays the registers stored on the stack, with the aid of
; .Names.Regs in Debug.inc.

Ints.Debug.Regs:

.Window         EQU             ((Debug.VGA.Top+Debug.Show.Height)*VGA.Cols+Debug.VGA.Left) * 2

                PUSH            CS              ; Point to Ints.Names
                POP             DS              ; With DS

                CLD
                MOV             ESI,Ints.Names.Regs ; Register Names only
.Regs:
                LODSB                           ; Get Debug offset
                MOVZX           ECX,AL          ; Into two registers
                MOV             EDX,ECX
                AND             ECX,3Fh         ; XLoc
                SHR             EDX,6           ; YLoc
                IMUL            EDX,VGA.Cols    ; RowLoc
                ADD             EDX,ECX         ; Combined

                MOV             AH,Debug.Colour.Label
                MOV             CL,Ints.Names.Label
                LEA             EDI,[.Window+EDX*2] ; Screen position
.Name:
                LODSB                           ; Get Name
                STOSW
                LOOP            .Name
                MOV             AL,':'          ; Separator
                STOSW

                LODSB                           ; Get Hex width
                MOV             CL,AL
                LODSB                           ; Get Stack offset
                INC             ESI             ; (Ignore TSS offset)
                MOVZX           EDX,AL          ; As indexable address

                MOV             AH,Debug.Colour.Regs
                MOV             EDX,[ESP+4+EDX] ; Value to display
                CALL            Ints.Hex

                CMP             ESI,Ints.Names.End
                JB              .Regs

                RET
