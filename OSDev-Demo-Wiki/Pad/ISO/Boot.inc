;
; Pad/ISO/Boot.inc
;

; This file defines the El Torito Boot Record Volume Sector for the ISO image

ISO.Sector.Boot       EQU               ISO.Sectors
%assign               ISO.Sectors       ISO.Sectors + 1

ISO.Volume.Boot:
.Header         ISO.Volume.Header       ISO.Volume.Header.Type.Boot, 1

.SystemID       DB              "EL TORITO SPECIFICATION"
                TIMES           ((26h-07h+1) - ($-.SystemID)) DB 00h

                TIMES           (46h-27h+1) DB 00h

.Catalog        DD              ISO.Sector.Catalog

.Padding        EQU             ISO.Sector.Size - ($ - ISO.Volume.Boot)
                TIMES           .Padding    DB 00h
