;
; Pad/ISO/PathTable.inc
;

; This file defines the ISO Path Table. There are two similar flavours: Big and
; Little Endian. To that end, this file expects the ISO.Endian macro to be set
; to either Little or Big before being %included.

ISO.Sector.PathTable.%[ISO.Endian] EQU  ISO.Sectors
%assign              ISO.Sectors        ISO.Sectors + 1

ISO.PathTable.%[ISO.Endian]:
.Length         DB              1       ; Number of Sectors in Path Table
.NumExtSectors  DB              0

.Sector         DD              ISO.Endian.%[ISO.Endian].DWord(ISO.Sector.Root)
.Parent         DW              ISO.Endian.%[ISO.Endian].Word(1)

.Name           DB              0
.Pad            DB              0

.Size           EQU             $ - ISO.PathTable.%[ISO.Endian]

.Padding        EQU             ISO.Sector.Size - .Size
                TIMES           .Padding    DB 00h
