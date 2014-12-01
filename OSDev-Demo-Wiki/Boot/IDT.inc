;
; Boot/IDT.inc
;

; This system's IDT is not fully populated - only the first 48 of 256 entries
; are used. Originally, I generated the IDT at run-time, but that used up a LOT
; of the boot sector. Since the IDT is not important until Protected Mode,
; loading it as part of the image makes sense, and is effectively free of
; run-time overhead (we're loading everything else anyway).
;
; But the result was 1,664 bytes of zeroes stored in the disk image. So this is
; the compromise: the used part of the IDT is stored in the image, then this
; code moves the IDT to its final resting home, zeroing out the unused part.

Boot.IDT:
                ; First, turn off hardware interrupts
                MOV             AL,0FFh
                OUT             Dev.PIC.A.Mask,AL
                OUT             Dev.PIC.B.Mask,AL

                MOV             AX,IDT.Start >> 4 ; Start segment
                MOV             DS,AX

                MOV             AX,IDT.Base >> 4 ; Destination segment
                MOV             ES,AX

                CLD                               ; Work forwards

                MOV             CX,(IDT.Limit + 1) / 4 ; Num DWORDs to move
                XOR             SI,SI             ; From the beginning
                XOR             DI,DI             ; Of both segments
                REP             MOVSD             ; Do it

                XOR             EAX,EAX           ; Now zero the rest of it
                MOV             CX,(IDT.Max - (IDT.Limit + 1)) / 4
                REP             STOSD             ; Do it
