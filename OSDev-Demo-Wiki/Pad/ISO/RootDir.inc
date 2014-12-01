;
; Pad/ISO/RootDir.inc
;

; This file defines the ISO Root directory. There's not much in it...

ISO.Sector.Root      EQU                ISO.Sectors
%assign              ISO.Sectors        ISO.Sectors + 1

ISO.RootDir:
.Dot            ISO.Dir.Root    ISO.Sector.Root, ISO.Dir.Flag.Dir, 1, 0
.DotDot         ISO.Dir.Root    ISO.Sector.Root, ISO.Dir.Flag.Dir, 1, 1

.Padding        EQU             ISO.Sector.Size - ($ - ISO.RootDir)
                TIMES           .Padding    DB 00h
