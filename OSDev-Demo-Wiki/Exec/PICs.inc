;
; Exec/PICs.inc
;

; To service hardware Interrupt ReQuests, the PICs need to be changed from their
; arbitrary BIOS-set Real Mode values to more rational settings. Other modules
; can then install IRQ handlers for their particular devices.
;
; To program a PIC, you first give it the Init Cmd, then you program the Mask
; register a few times in a prescribed order. The details change depending on
; whether the PIC is a Master or a Slave.

Exec.PICs:
                MOV             AL,Dev.PIC.Cmd.Init     ; Initialise PICs
                OUT             Dev.PIC.A.Cmd,AL
                OUT             Dev.PIC.B.Cmd,AL

                MOV             AL,PIC.A.Base           ; Set PIC.A's Base
                OUT             Dev.PIC.A.Mask,AL
                MOV             AL,PIC.B.Base           ; Set PIC.B's Base
                OUT             Dev.PIC.B.Mask,AL

                MOV             AL,Dev.PIC.A.Cascade    ; Tell PIC.A where Slave is
                OUT             Dev.PIC.A.Mask,AL

                MOV             AL,Dev.PIC.B.Cascade    ; Tell PIC.B where it is Slave
                OUT             Dev.PIC.B.Mask,AL

                MOV             AL,Dev.PIC.Init.8086    ; 80x86 mode
                OUT             Dev.PIC.A.Mask,AL
                OUT             Dev.PIC.B.Mask,AL

                MOV             AL,~Dev.PIC.A.Cascade   ; Allow Cascade interrupts through
                OUT             Dev.PIC.A.Mask,AL
                MOV             AL,0FFh                 ; Mask all interrupts
                OUT             Dev.PIC.B.Mask,AL

                RET
