;
; Pad/ISO/Defn.inc
;

; This file holds the definitions for the ISO standard. The biggest oddity is
; their way of catering for both little-endian and big-endian systems out there:
; they avoid it altogether and use a both-endian double-sized structure!

; The following have been defined by the ISO standard
ISO.Sector.Size EQU             2048

ISO.Boot.Media.NoEmulation EQU  0
ISO.Boot.Media.Floppy.1200 EQU  1
ISO.Boot.Media.Floppy.1440 EQU  2
ISO.Boot.Media.Floppy.2880 EQU  3
ISO.Boot.Media.HardDisk    EQU  4

ISO.Boot.Platform.x86      EQU  0
ISO.Boot.Platform.PowerPC  EQU  1
ISO.Boot.Platform.Mac      EQU  2

; For some fields, I need a date and time. Why not now?
%defstr         ISO.Now.Date    __DATE_NUM__    ; YYYYMMDD
%defstr         ISO.Now.Time    __TIME_NUM__    ; HHMMSS

; This extracts sub-parts of the above numbers as integers
%define         ISO.Extract(Number, Mod, Div) ((Number % Mod) / Div)

; The ISO standard caters for both Big- and Little-endian numbers. Here are
; some macros for when it matters which will be used
%define         ISO.Endian.Little.Word(Num)  (Num)
%define         ISO.Endian.Big.Word(Num)     ((Num >> 8) | (Num << 8) & 0FFFFh)

%define         ISO.Endian.Little.DWord(Num) (Num)
%define         ISO.Endian.Big.DWord(Num)    ( ((Num >> 24) &  0000_00FFh) | \
                                               ((Num >>  8) &  0000_FF00h) | \
                                               ((Num <<  8) &  00FF_0000h) | \
                                               ((Num << 24) & 0FF00_0000h)   \
                                             )

; And here are the macros to represent the Both-endian numbers
%macro          ISO.Endian.Word 1
                DW              ISO.Endian.Little.Word(%1)
                DW              ISO.Endian.Big.Word(%1)
%endmacro

%macro          ISO.Endian.DWord 1
                DD              ISO.Endian.Little.DWord(%1)
                DD              ISO.Endian.Big.DWord(%1)
%endmacro

; NASM's __DATE_NUM__ and __TIME_NUM__ macros are already in the correct format!
%macro          ISO.DateTime    0
%00.Date        DB              ISO.Now.Date
%00.Time        DB              ISO.Now.Time
%00.Hundredths  DB              "00"
%00.Timezone    DB              ISO.Timezone
%if ($-%00.Date) != 17
%error "DateTime macro results incorrect"
%endif
%endmacro

; Every Volume sector in the Volume Set needs the same Header to start
%define         ISO.Volume.Header.ID.String "CD001"

ISO.Volume.Header.Type.Boot         EQU     0
ISO.Volume.Header.Type.Primary      EQU     1
ISO.Volume.Header.Type.Supplemetary EQU     2
ISO.Volume.Header.Type.Partition    EQU     3
ISO.Volume.Header.Type.Terminator   EQU   255

                STRUC           ISO.Volume.Header
.Type           RESB            1               ; ISO.Volume.Header.Type.*
.ID             RESB            5               ; ISO.Volume.Header.ID.String
.Version        RESB            1               ; Usually 1
                ENDSTRUC

%macro          ISO.Volume.Header 2
%00             ISTRUC        ISO.Volume.Header
AT ISO.Volume.Header.Type,    DB                %1
AT ISO.Volume.Header.ID,      DB                ISO.Volume.Header.ID.String
AT ISO.Volume.Header.Version, DB                %2
                IEND
%endmacro

; This is the format of an ISO Directory Entry. At least, it nearly is -
; .Identifier is the name, and the whole Entry's size needs to be even
                STRUC           ISO.Dir
.NumBytes       RESB            1
.NumExtSectors  RESB            1
.Sector         RESD            2       ; Both-endian
.Size           RESD            2       ; Both-endian
.Years          RESB            1
.Month          RESB            1
.Day            RESB            1
.Hour           RESB            1
.Min            RESB            1
.Sec            RESB            1
.Timezone       RESB            1
.Flags          RESB            1
.Interleave.Size RESB           1
.Interleave.Gap RESB            1
.Volume.SeqNum  RESW            2       ; Both-endian
.Length         RESB            1
.Identifier     RESB            1
                ENDSTRUC

; These are the most common Entry flags
ISO.Dir.Flag.Normal EQU         0000_0000b
ISO.Dir.Flag.Hidden EQU         0000_0001b
ISO.Dir.Flag.File   EQU         0000_0000b
ISO.Dir.Flag.Dir    EQU         0000_0010b

%macro          ISO.Dir.Root    4
%00             ISTRUC    ISO.Dir
AT ISO.Dir.NumBytes,      DB    ISO.Dir_size
AT ISO.Dir.NumExtSectors, DB    0
AT ISO.Dir.Sector,        ISO.Endian.DWord %1
AT ISO.Dir.Size,          ISO.Endian.DWord ISO.Sector.Size
AT ISO.Dir.Years,         DB    ISO.Extract(__DATE_NUM__,100000000,10000) - 1900
AT ISO.Dir.Month,         DB    ISO.Extract(__DATE_NUM__,10000,100)
AT ISO.Dir.Day,           DB    ISO.Extract(__DATE_NUM__,100,1)
AT ISO.Dir.Hour,          DB    ISO.Extract(__TIME_NUM__,1000000,10000)
AT ISO.Dir.Min,           DB    ISO.Extract(__TIME_NUM__,10000,100)
AT ISO.Dir.Sec,           DB    ISO.Extract(__TIME_NUM__,100,1)
AT ISO.Dir.Timezone,      DB    ISO.Timezone
AT ISO.Dir.Flags,         DB    %2
AT ISO.Dir.Interleave.Size, DB  0
AT ISO.Dir.Interleave.Gap,  DB  0
AT ISO.Dir.Volume.SeqNum, ISO.Endian.Word 1
AT ISO.Dir.Length,        DB    %3
AT ISO.Dir.Identifier,    DB    %4
                IEND
%endmacro

; The Boot Catalog has the only Checksum in the formal ISO specification.
; Here are the macro definitions to get NASM to calculate it for us!
ISO.Validation.Header    EQU    01h
ISO.Validation.Signature EQU    0AA55h

%macro          ISO.Checksum    2
%assign         Checksum        (%2<<8) + ISO.Validation.Header + ISO.Validation.Signature
%strlen         ID.Length       %1
%assign         ID.Pos          1
%rep ID.Length
%substr         ID.High         %1 ID.Pos+1
%substr         ID.Low          %1 ID.Pos+0
%assign         Checksum        Checksum + ((ID.High << 8) | ID.Low)
%assign         ID.Pos          ID.Pos+2
%endrep
%00             DW              (0-Checksum) & 0FFFFh
%endmacro

%macro          ISO.Validation  2
.Header         DB              ISO.Validation.Header
.Platform       DB              %2
.Reserved       DW              0000h
.ID             DB              %1
                TIMES           ((1Bh-04h+1) - ($-.ID)) DB 00h
.Checksum       ISO.Checksum    %1, %2
.Signature      DW              ISO.Validation.Signature
%endmacro
