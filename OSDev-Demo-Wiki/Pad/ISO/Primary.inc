;
; Pad/ISO/Primary.inc
;

; This file defines the Primary Volume Sector for the ISO image.

%define         Version.System          Version.Program, " ", Version.Copyright, " v.", Version.String

%define         ISO.Primary.System      Version.System
%define         ISO.Primary.Volume      Version.Name
%define         ISO.Primary.VolumeSet   ""
%define         ISO.Primary.Publisher   Version.Author
%define         ISO.Primary.Preparer    Version.Author
%define         ISO.Primary.Application "NASM (http://www.nasm.us/) ", \
                                        __NASM_VER__, " ", __DATE__, " ", __TIME__
%define         ISO.Primary.Copyright   ""      ; Name of file in Root
%define         ISO.Primary.Abstract    ""      ; Name of file in Root
%define         ISO.Primary.Biblio      ""      ; Name of file in Root

ISO.Sector.Primary    EQU               ISO.Sectors
%assign               ISO.Sectors       ISO.Sectors + 1

ISO.Volume.Primary:
.Header         ISO.Volume.Header       ISO.Volume.Header.Type.Primary, 1
                DB              00h

.SystemID       DB              ISO.Primary.System ; (Uppercase and punctuation)
                TIMES           (32-($-.SystemID)) DB ' '

.VolumeID       DB              ISO.Primary.Volume
                TIMES           (32-($-.VolumeID)) DB ' ' ; (Uppercase only)

                TIMES           8 DB 00h

.TotalSectors   ISO.Endian.DWord ISO.Sector.Last

                TIMES            32 DB 00h

.VolumeSetSize  ISO.Endian.Word  1              ; Number of Volumes in this Set
.VolumeSeqNum   ISO.Endian.Word  1              ; Which number Volume this is
.LogicalSize    ISO.Endian.Word  ISO.Sector.Size

.PathTableSize.Little DD         ISO.Endian.Little.DWord(ISO.PathTable.Little.Size)
.PathTableSize.Big    DD         ISO.Endian.Big.DWord(ISO.PathTable.Big.Size)
.PathTable.1.Little   DD         ISO.Endian.Little.DWord(ISO.Sector.PathTable.Little)
.PathTable.2.Little   DD         ISO.Endian.Little.DWord(0) ; Optional, so blank
.PathTable.1.Big      DD         ISO.Endian.Big.DWord(ISO.Sector.PathTable.Big)
.PathTable.2.Big      DD         ISO.Endian.Big.DWord(0)    ; Optional, so blank

.RootDirectory  ISO.Dir.Root     ISO.Sector.Root, ISO.Dir.Flag.Dir, 1, 0

.VolumeSetID    DB               ISO.Primary.VolumeSet ; (Uppercase only)
                TIMES            (128-($-.VolumeSetID)) DB ' '

.PublisherID    DB               ISO.Primary.Publisher ; (Uppercase and punctuation)
                TIMES            (128-($-.PublisherID)) DB ' '

.PreparerID     DB               ISO.Primary.Preparer ; (Uppercase and punctuation)
                TIMES            (128-($-.PreparerID)) DB ' '

.ApplicationID  DB               ISO.Primary.Application ; (Uppercase and punctuation)
                TIMES            (128-($-.ApplicationID)) DB ' '

.CopyrightFile  DB               ISO.Primary.Copyright
                TIMES            (37-($-.CopyrightFile)) DB ' '

.AbstractFile   DB               ISO.Primary.Abstract
                TIMES            (37-($-.AbstractFile)) DB ' '

.BiblioFile     DB               ISO.Primary.Biblio
                TIMES            (37-($-.BiblioFile)) DB ' '

.CreateDate     ISO.DateTime
.ModifyDate     ISO.DateTime
.ExpiryDate     DB               "9999123123595999",-48 ; Basically never...
.EffectiveDate  ISO.DateTime

.StructureVer   DB               01h
.Reserved       DB               00h

.PrivateUse     TIMES            512 DB 00h

.Padding        EQU             ISO.Sector.Size - ($ - ISO.Volume.Primary)
                TIMES           .Padding    DB 00h
