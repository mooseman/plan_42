;
; BIOS/VGA.inc
;

; These are the definitions for the BIOS video controller

BIOS.VGA.Int    EQU             10h

BIOS.VGA.SetMode.Fn    EQU      00h
BIOS.VGA.SetMode.80x25 EQU      03h

BIOS.VGA.ExtMode.Fn    EQU      11h
BIOS.VGA.ExtMode.80x50 EQU      12h

BIOS.VGA.Cursor.Fn    EQU       01h
BIOS.VGA.Cursor.None  EQU       0010_0000_0000_0000b
BIOS.VGA.Cursor.Under EQU       06_07h
BIOS.VGA.Cursor.Block EQU       00_07h
