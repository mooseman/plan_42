;
; Dev/PIC.inc
;

; These are the definitions for the Priority Interrupt Controllers
Dev.PIC.A.Cmd   EQU             020h
Dev.PIC.A.Mask  EQU             021h

Dev.PIC.B.Cmd   EQU             0A0h
Dev.PIC.B.Mask  EQU             0A1h

Dev.PIC.Cmd.EOI   EQU           20h
Dev.PIC.Cmd.IRR   EQU           0Ah             ; Access Interrupt Request Register
Dev.PIC.Cmd.ISR   EQU           0Bh             ; Access In-Service Register

Dev.PIC.Cmd.Init  EQU           11h
Dev.PIC.Init.8086 EQU           01h

Dev.PIC.A.Cascade EQU           0000_0100b      ; PIC.A has slave on IRQ2
Dev.PIC.B.Cascade EQU           02h             ; PIC.B is slaved on IRQ2

Dev.PIC.A.Timer   EQU           0000_0001b
Dev.PIC.A.Key     EQU           0000_0010b
Dev.PIC.A.COM2    EQU           0000_1000b
Dev.PIC.A.COM1    EQU           0001_0000b
Dev.PIC.A.Floppy  EQU           0100_0000b
Dev.PIC.A.Printer EQU           1000_0000b

Dev.PIC.B.RTC     EQU           0000_0001b
Dev.PIC.B.Mouse   EQU           0001_0000b
Dev.PIC.B.IDE0    EQU           0100_0000b
Dev.PIC.B.IDE1    EQU           1000_0000b
