;
; Boot/Key.inc
;

; Use the BIOS to speed up the keyboard to as fast as it will go

Key.SetRate     EQU             (BIOS.Key.SetRate.Fn << 8) | BIOS.Key.SetRate.Set
Key.Rate        EQU             (BIOS.Key.SetRate.Delay.250 << 8) | BIOS.Key.SetRate.Rate.300

Boot.Key:
; The following doesn't work as intended under VMware
;               MOV             AX,Key.SetRate
;               MOV             BX,Key.Rate
;               INT             BIOS.Key.Int
