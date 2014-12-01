;
; Boot/A20.inc
;

; The 8086 has a 20-bit address bus. If you loaded a segment register with
; 0FFFFh and accessed 0FFFFh:0FFFFh, the Segment:Offset addition would wrap
; and you'd actually be accessing 0000_FFEFh. The BIOS would "take advantage"
; of this quirk and use it to access both top-of-Address BIOS and botttom-of-
; Address RAM with the same segment register.
;
; Of course, then the '286 PC/AT came out with a 24-bit bus - and the wrap no
; longer occurred. You could use it to access the (nearly) 64K of High Memory!
; But what about all the legacy code? That was now broken.
;
; To "fix" this, the designers of the PC/AT added a Gate to the A20 line, to
; hold A20 on the memory bus at zero regardless of what the CPU wanted. This
; emulated the wrap-around "feature", but made the high memory inaccessible.
;
; So to fix that, they found a spare output that they could program at run-time
; to turn the Gate on or off. Where was this spare pin? The keyboard controller,
; of course...
;
; As PCs became more sophisticated, the A20 Gate stayed, but the programming
; mechanism to turn it on or off changed - and it wasn't really standardised.
; Since this program wants to access high memory, it has to turn off the Gate -
; and it uses a number of techniques to attempt this.
;
; Of course, this means finding out whether A20 is off, both up front and after
; each technique. How? Go to a known memory location, get its value, and compare
; it with its high-memory "alias". If they're different, then the Gate is off.
; If they're the same, then maybe it was a coincidence? Change the original
; location, and check the alias again. If they're now different (they were the
; same, but now they're different), the Gate must be off.

A20.Enable EQU             (BIOS.A20.Fn << 8) | BIOS.A20.Enable

; [0000h:BIOS.Entry + BIOS.Sig.Pos] holds the value BIOS.Sig.Value
; [FFFFh:BIOS.Entry + BIOS.Sig.Pos + 10h] is its alias, if the A20 Gate is on.
Sig.Alias  EQU             BIOS.Entry + BIOS.Sig.Pos + 10h

Boot.A20:
; CS:BIOS.Sig.Pos has a known, safely-modifable value.
; Prepare for testing, by comparing CS:BIOS.Sig.Pos with 0FFFFh:Sig.Alias
                MOV             SI,BIOS.Sig.Pos

                MOV             AX,0FFFFh       ; End of 1 MB
                MOV             ES,AX           ; In ES
                MOV             DI,Sig.Alias

; First things first: has it already been done for us?
                CALL            .Test           ; Test if A20 already on
                JE              .End            ; Yep! Nothing to do

; No. OK, maybe the BIOS can help.
                MOV             AX,A20.Enable
                INT             BIOS.A20.Int    ; Using BIOS

                CALL            .Test           ; Did it work?
                JE              .End            ; Yep! Nothing more to do

; No. OK: the traditional ways are always the best?
                CALL            Boot.A20.Switch ; Switch A20 myself
;               JNZ             $               ; Uh oh! ***STOP!***

                CALL            .Test           ; Did it work?
                JE              .End            ; Yep! Nothing more to do

; No. Maybe it's a PS/2?
                IN              AL,Dev.A20.Fast.Port ; Try Fast A20
                TEST            AL,Dev.A20.Fast.A20 ; Enabled?
                JNZ             $               ; Uh oh! Isn't working! ***STOP!***
                OR              AL,Dev.A20.Fast.A20    ; Enable it
                AND             AL,~Dev.A20.Fast.Reset ; But NOT Fast Reset!
                OUT             Dev.A20.Fast.Port,AL

                CALL            .Test           ; Did it work?
                JE              .End            ; Yep! Finally!

                JMP             $               ; No reaction! ***STOP!***
Boot.A20.Test:
                MOV             DX,[CS:SI]      ; Get current value
                CMP             DX,[ES:DI]      ; Same at end of RAM?
                JNE             .Good           ; No, so no problem!

                NOT     WORD    [CS:SI]         ; Change current value
                CMP             DX,[ES:DI]      ; Still same as old?
                JMP             .Bad            ; No, so problem!
.Good:
                XOR             AX,AX           ; Set Z flag
.Bad:
                RET

;-------------------------------------------------------------------------------

Boot.A20.Switch:
                CALL            .Wait          ; Wait for 8042 to be ready
                JNZ             .Dead          ; Uh oh...

                MOV             AL,Dev.A20.Ctrl.Write
                OUT             Dev.A20.Ctrl.Port,AL ; Tell 8042 we're writing to it
                CALL            .Wait          ; Wait for acknowledge
                JNZ             .Dead          ; Uh oh...

                MOV             AL,Dev.A20.Data.Enable
                OUT             Dev.A20.Data.Port,AL ; Tell 8042 to Enable A20
;...............................................................................
.Wait:
                XOR             CX,CX           ; Try a LARGE number of times!
.Loop:
                IN              AL,Dev.A20.Stat.Port ; Wait to write
                TEST            AL,Dev.A20.Stat.InFull ; YES, In! It's backwards
                LOOPNZ          .Loop           ; Nope. Still full
.Dead:
                RET

;-------------------------------------------------------------------------------

Boot.A20.End:
