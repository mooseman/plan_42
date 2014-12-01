;
; BIOS/Key.inc
;

; These are the definitions for the BIOS keyboard controller

BIOS.Key.Int    EQU             16h

BIOS.Key.SetRate.Fn       EQU   03h
BIOS.Key.SetRate.Default  EQU   00h ; Set back to default
BIOS.Key.SetRate.Increase EQU   01h
BIOS.Key.SetRate.Decrease EQU   02h ; Slow down by half
BIOS.Key.SetRate.Off      EQU   04h ; Turn off
BIOS.Key.SetRate.Set      EQU   05h ; Set to below values (BH:BL)

; Delay before typematic starts, in milliseconds (in BH)
BIOS.Key.SetRate.Delay.250  EQU 00h
BIOS.Key.SetRate.Delay.500  EQU 01h
BIOS.Key.SetRate.Delay.750  EQU 02h
BIOS.Key.SetRate.Delay.1000 EQU 03h

; Rate at which typematic occurs, in chars per 10 seconds (in BL)
BIOS.Key.SetRate.Rate.300  EQU  00h
BIOS.Key.SetRate.Rate.267  EQU  01h
BIOS.Key.SetRate.Rate.240  EQU  02h
BIOS.Key.SetRate.Rate.218  EQU  03h
BIOS.Key.SetRate.Rate.200  EQU  04h
BIOS.Key.SetRate.Rate.185  EQU  05h
BIOS.Key.SetRate.Rate.171  EQU  06h
BIOS.Key.SetRate.Rate.160  EQU  07h
BIOS.Key.SetRate.Rate.150  EQU  08h
BIOS.Key.SetRate.Rate.133  EQU  09h
BIOS.Key.SetRate.Rate.120  EQU  0Ah
BIOS.Key.SetRate.Rate.109  EQU  0Bh
BIOS.Key.SetRate.Rate.100  EQU  0Ch
BIOS.Key.SetRate.Rate.92   EQU  0Dh
BIOS.Key.SetRate.Rate.86   EQU  0Eh
BIOS.Key.SetRate.Rate.80   EQU  0Fh
BIOS.Key.SetRate.Rate.75   EQU  10h
BIOS.Key.SetRate.Rate.67   EQU  11h
BIOS.Key.SetRate.Rate.60   EQU  12h
BIOS.Key.SetRate.Rate.55   EQU  13h
BIOS.Key.SetRate.Rate.50   EQU  14h
BIOS.Key.SetRate.Rate.46   EQU  15h
BIOS.Key.SetRate.Rate.43   EQU  16h
BIOS.Key.SetRate.Rate.40   EQU  17h
BIOS.Key.SetRate.Rate.37   EQU  18h
BIOS.Key.SetRate.Rate.33   EQU  19h
BIOS.Key.SetRate.Rate.30   EQU  1Ah
BIOS.Key.SetRate.Rate.27   EQU  1Bh
BIOS.Key.SetRate.Rate.25   EQU  1Ch
BIOS.Key.SetRate.Rate.23   EQU  1Dh
BIOS.Key.SetRate.Rate.21   EQU  1Eh
BIOS.Key.SetRate.Rate.20   EQU  1Fh
