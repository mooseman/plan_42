;
; Ints/Key.inc
;

; This module implements the Keyboard interrupt handler.
; It simply gets the scan-code for the key and stores it into Data.Key.Code.
; This is a one-byte buffer - if you miss it before the next one, it's gone!
; Of course, not much is happening on this system, so it's unlikely it'll get
; missed.
;
; Note that the value stored is a scan-code, which is different from what you
; get from a getc() call - that's ASCII. Scan-codes are whatever the keyboard
; wants to generate for each key, and there's nothing obvious about their
; arrangement - so I've added code that displays the Hex value of the scan-code
; on the screen on every interrupt.
;
; Also note that the keyboard generates one interrupt when the key is pressed,
; and a second (with the same scan-code but with the high bit set) when the key
; is released. In this way the keyboard handler can keep track of which keys are
; currently being held down. If the key isn't released after a short while,
; another scan-code will be generated, again with the high bit clear.
;
; What isn't obvious with the display code as it is, is that for some keys there
; are two (or more) interrupts generated. The original PC had one set of number
; and arrow keys, while the Extended keyboard duplicated these with a keypad.
; For older systems these new keys had to generate the same scan-codes as the
; old ones, but newer systems wanted to distinguish between the two. So the new
; keys were designed to prefix the old scan-code with a "magic" keypress that
; the old systems didn't know about, so simply ignored.
;
; Needless to say, extending the functionality is left as an exercise for the
; reader!

Ints.Key:
                PUSH            EAX             ; Need these registers
                PUSH            ES              ; For .Hex's STOSW later...

                ; Point to Global Data
                MOV             AX,Selector(GDT.Data, GDT, RPL0)
                MOV             ES,AX

                IN              AL,Dev.Key.Data ; Get character from keyboard
                MOV             [ES:Data.Key.Code],AL ; Store in global Data

                MOV             AH,AL            ; Save to show

                MOV             AL,Dev.PIC.Cmd.EOI ; Can now acknowledge PIC
                OUT             Dev.PIC.A.Cmd,AL

                CMP             AH,Dev.Key.Pause ; Also labeled "Break"...
                JNE             .Continue        ; Don't Break?
                STI                              ; Do! So need interrupts back on
                DEBUG                            ; (Debugger uses keyboard)
.Continue:
                CALL            .Show           ; Show scan-code on screen

                POP             ES              ; That was easy!
                POP             EAX
                IRETD

;-------------------------------------------------------------------------------
; This function shows the scan-code (in AL) on the screen

.Show:

; .Key.Pos is chosen as the keyboard IRQ's screen position - but the Hex is two
; characters wide. Luckily, the next IRQ (IRQ2) is the Cascade, so it never fires!
.Key.Pos        EQU             33 * 2          ; Location on screen to show Key
.Key.Colour     EQU             Dev.VGA.CyanBack | Dev.VGA.Black

                PUSH            ECX             ; Need these
                PUSH            EDX
                PUSH            EDI  

                ; Now point to screen
                MOV             DI,Selector(GDT.VGA, GDT, RPL0)
                MOV             ES,DI

                MOV             DL,AH           ; Code to Hexify
                MOV             AH,.Key.Colour  ; Colour to use
                MOV             ECX,2           ; Only 2 digits
                MOV             EDI,.Key.Pos    ; Position to show it
                CALL            Ints.Hex

                POP             EDI             ; Done!
                POP             EDX
                POP             ECX
                RET
