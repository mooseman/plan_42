;
; Dev/A20.inc
;

; These are the definitions for the A20 Gate registers
Dev.A20.Fast.Port  EQU          092h
Dev.A20.Fast.A20   EQU          0000_0010b      ; Set this bit to Enable A20
Dev.A20.Fast.Reset EQU          0000_0001b      ; Set this bit to Reset PC

Dev.A20.Ctrl.Port  EQU          064h
Dev.A20.Ctrl.Read  EQU          0D0h
Dev.A20.Ctrl.Write EQU          0D1h

Dev.A20.Stat.Port  EQU          064h
Dev.A20.Stat.InFull  EQU        0000_0010b
Dev.A20.Stat.OutFull EQU        0000_0001b

Dev.A20.Data.Port  EQU          060h
Dev.A20.Data.Enable EQU         0DFh
