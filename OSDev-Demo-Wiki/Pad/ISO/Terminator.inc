;
; Pad/ISO/Terminator.inc
;

; This file defines the Set Terminator Volume Sector for the ISO image

ISO.Sector.Terminator EQU               ISO.Sectors
%assign               ISO.Sectors       ISO.Sectors + 1

ISO.Volume.Terminator:
.Header         ISO.Volume.Header       ISO.Volume.Header.Type.Terminator, 1

.Padding        EQU             ISO.Sector.Size - ($ - ISO.Volume.Terminator)
                TIMES           .Padding    DB 00h
