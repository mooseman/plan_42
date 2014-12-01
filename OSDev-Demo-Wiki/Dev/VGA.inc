;
; Dev/VGA.inc
;

; These are the definitions for the Video Graphics Array (VGA) Controller.
;
; The memory arrangement of the Text mode of the IBM PC is a linear array of
; two-byte locations. The first memory location is the top left of the screen,
; followed by 79 (typically) more locations for the first row, immediately
; followed by the memory for the second row, until all the rows are displayed.
; Each screen location is two bytes:
;
; The first byte is the character code, which (luckily) is ASCII for the
; characters #32-#126 - most text is easy to display. The other codes in the
; #0-#255 range are often called "OEM" codes - different countries use different
; codes. The default mapping is used in this system.
;
; The second byte is that character's attribute - typically its colour. The low
; nybble is the foreground colour, while the high nybble is the background
; colour - except for the highest bit, which is often set to flash the
; character. Many systems though don't do this, instead substituting a wider
; range of shades of colours.
;
; The colours available to display are essentially the primary RGB and the
; secondary CMY, with the necessary White and Black added in. The table for the
; default colour mappings is provided below.

Dev.VGA.Base      EQU           000B_8000h      ; Base memory address

Dev.VGA.Foregruond EQU          0000_1111b
Dev.VGA.Bright     EQU          0000_1000b
Dev.VGA.Background EQU          0111_0000b
Dev.VGA.Blink      EQU          1000_0000b

Dev.VGA.Black     EQU           0_0h
Dev.VGA.Blue      EQU           0_1h
Dev.VGA.Green     EQU           0_2h
Dev.VGA.Cyan      EQU           0_3h
Dev.VGA.Red       EQU           0_4h
Dev.VGA.Magenta   EQU           0_5h
Dev.VGA.Brown     EQU           0_6h
Dev.VGA.Grey      EQU           0_7h

Dev.VGA.Dark          EQU       Dev.VGA.Bright | Dev.VGA.Black
Dev.VGA.BrightBlue    EQU       Dev.VGA.Bright | Dev.VGA.Blue
Dev.VGA.BrightGreen   EQU       Dev.VGA.Bright | Dev.VGA.Green
Dev.VGA.BrightCyan    EQU       Dev.VGA.Bright | Dev.VGA.Cyan
Dev.VGA.BrightRed     EQU       Dev.VGA.Bright | Dev.VGA.Red
Dev.VGA.BrightMagenta EQU       Dev.VGA.Bright | Dev.VGA.Magenta
Dev.VGA.Yellow        EQU       Dev.VGA.Bright | Dev.VGA.Brown
Dev.VGA.White         EQU       Dev.VGA.Bright | Dev.VGA.Grey

Dev.VGA.BlackBack     EQU       Dev.VGA.Black   << 4
Dev.VGA.BlueBack      EQU       Dev.VGA.Blue    << 4
Dev.VGA.GreenBack     EQU       Dev.VGA.Green   << 4
Dev.VGA.CyanBack      EQU       Dev.VGA.Cyan    << 4
Dev.VGA.RedBack       EQU       Dev.VGA.Red     << 4
Dev.VGA.MagentaBack   EQU       Dev.VGA.Magenta << 4
Dev.VGA.YellowBack    EQU       Dev.VGA.Brown   << 4
Dev.VGA.WhiteBack     EQU       Dev.VGA.Grey    << 4

Dev.VGA.Misc.Port EQU           3C2h            ; VGA miscellaneous port

Dev.VGA.Seq.Base  EQU           3C4h            ; VGA Sequencer base port

Dev.VGA.CRTC.Base EQU           3D4h            ; VGA CRTC base port
