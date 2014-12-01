;
; Boot/VGA.inc
;

; Use the BIOS to set the desired video mode (80x50 by default: 90x60 if enabled)

VGA.Set80x25    EQU             (BIOS.VGA.SetMode.Fn << 8) | BIOS.VGA.SetMode.80x25
VGA.Ext80x50    EQU             (BIOS.VGA.ExtMode.Fn << 8) | BIOS.VGA.ExtMode.80x50

; Note: DS and ES are 0000h
Boot.VGA:
                MOV             AX,VGA.Set80x25 ; Set 80x25 mode, 8x16 font
                INT             BIOS.VGA.Int

                MOV             AX,VGA.Ext80x50 ; Ext 80x50 mode, 8x8 font
                MOV             BL,0
                INT             BIOS.VGA.Int

%ifndef BIOS.VGA.90x60

VGA.Rows        EQU             50
VGA.Cols        EQU             80

                MOV             AH,BIOS.VGA.Cursor.Fn   ; Set cursor size
                MOV             CX,BIOS.VGA.Cursor.None ; To invisible
                INT             BIOS.VGA.Int

%else ; BIOS.VGA.90x60

VGA.Rows        EQU             60
VGA.Cols        EQU             90

;-------------------------------------------------------------------------------

; I know I said I don't like "magic numbers", but the following is esoteric!
                MOV             AL,0E7h
                MOV             DX,Dev.VGA.Misc.Port
                OUT             DX,AL

                CLD                             ; Work forwards
                MOV             SI,Boot.VGA.Seq

                MOV             CX,.Seq.Num     ; This many Seq registers
                MOV             DX,Dev.VGA.Seq.Base
                CALL            Boot.VGA.Write

                MOV             AL,017h         ; Enable regs 0-7
                MOV             DX,Dev.VGA.CRTC.Base
                OUT             DX,AL
                INC             DX              ; Go to Data port
                IN              AL,DX
                AND             AL,~80h         ; Enable registers
                OUT             DX,AL
                DEC             DX              ; Back to Index port

                MOV             AL,003h         ; Enable vertical retrace
                OUT             DX,AL
                INC             DX              ; Go to Data port
                IN              AL,DX
                OR              AL,80h          ; Enable registers
                OUT             DX,AL
                DEC             DX              ; Back to Index port

                MOV             CX,.CRTC.Num    ; This many CRTC registers
                PUSH            Boot.VGA.End    ; Fake a call from .End
;-------------------------------------------------------------------------------

.Write:
                MOV             AH,0            ; Start with register 0
.Write.Loop:
                CS                              ; Load from Code Segment!
                LODSB
                XCHG            AL,AH           ; Make sure Index is in AL
                OUT             DX,AX           ; 16-bit write
                MOV             AH,AL           ; But need AL for LODSB
                INC             AH              ; Next index
                LOOP            .Write.Loop
                RET

.Seq            DB              003h, 001h, 003h, 000h, 002h ; Sequence registers
.Seq.Num        EQU             $-.Seq
.CRTC           DB              06Bh, 059h, 05Ah, 082h, 060h, 08Dh, 00Bh, 03Eh
                DB              000h, 047h, 020h, 000h, 000h, 000h, 000h, 000h
                DB              0EAh, 00Ch, 0DFh, 02Dh, 008h, 0E8h, 005h, 0A3h
                DB              0FFh                         ; CRTC registers
.CRTC.Num       EQU             $-.CRTC

%endif ; BIOS.VGA.90x60

.End:
