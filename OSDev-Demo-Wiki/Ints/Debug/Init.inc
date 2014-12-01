;
; Ints/Debug/Init.inc
;

; This module handles the Debug interrupt Init and Done routines.

;...............................................................................
Ints.Debug.Init:
                MOV             AX,Selector(GDT.VGA, GDT, RPL0)
                MOV             ES,AX           ; Point to Screen, for STOSW
                MOV             AX,Selector(GDT.Data, GDT, RPL0)
                MOV             FS,AX           ; Point to global (Key) Data

                ; Default Show.Row format
                CALL            Ints.Debug.Key.Format.Bytes
                CALL            Ints.System.Init
                RET

;...............................................................................
Ints.Debug.Done:
                CLD                             ; Work forwards
                MOV             AH,Debug.Colour.Blank
                MOV             AL,' '
                MOV             DH,Debug.VGA.Height
                MOV             EDI,(Debug.VGA.Top*VGA.Cols+Debug.VGA.Left)*2
.Loop:
                MOV             ECX,Debug.VGA.Width
                REP             STOSW
                ADD             EDI,(VGA.Cols-Debug.VGA.Width)*2
                DEC             DH
                JNZ             .Loop

                CALL            Ints.System.Done
                RET
