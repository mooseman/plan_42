;
; Dev/Key.inc
;

; These are the definitions for the Keyboard

Dev.Key.Data    EQU             060h
Dev.Key.Ctrl    EQU             064h

Dev.Key.Make    EQU             000h            ; "Make" codes have high bit off
Dev.Key.Break   EQU             080h            ; "Break" codes have high bit on

Dev.Key.Ext1    EQU             0E0h            ; This is the Extended Key prefix
Dev.Key.Ext2    EQU             0E1h            ; This one means TWO following

; Scan codes for various keys
Dev.Key.Esc     EQU             01h

Dev.Key.Enter   EQU             1Ch
Dev.Key.Space   EQU             39h

Dev.Key.Pause   EQU             45h

Dev.Key.Home    EQU             47h
Dev.Key.Up      EQU             48h
Dev.Key.PgUp    EQU             49h

Dev.Key.Left    EQU             4Bh
Dev.Key.Right   EQU             4Dh

Dev.Key.End     EQU             4Fh
Dev.Key.Down    EQU             50h
Dev.Key.PgDn    EQU             51h

Dev.Key.Ins     EQU             52h
Dev.Key.Del     EQU             53h

Dev.Key.F1      EQU             3Bh
Dev.Key.F2      EQU             3Ch
Dev.Key.F3      EQU             3Dh
Dev.Key.F4      EQU             3Eh
Dev.Key.F5      EQU             3Fh
Dev.Key.F6      EQU             40h
Dev.Key.F7      EQU             41h
Dev.Key.F8      EQU             42h
Dev.Key.F9      EQU             43h
Dev.Key.F10     EQU             44h
Dev.Key.F11     EQU             57h
Dev.Key.F12     EQU             58h

Dev.Key.Window  EQU             5Bh
Dev.Key.Menu    EQU             5Dh
