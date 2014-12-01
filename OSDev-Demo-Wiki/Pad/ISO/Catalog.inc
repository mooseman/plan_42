;
; Pad/ISO/Catalog.inc
;

; This file defines the Boot Catalog Sector for the ISO image. This basically
; points to the first sector, which is where the standard boot sector is anyway.
;
; Note that ISO.Boot.Default.Media has been set to Floppy.1440. This option
; works for VMware, VirtualPC and VirtualBox. Other options work for VMware only

%define         ISO.Catalog.ID          Version.Author

ISO.Sector.Catalog   EQU                ISO.Sectors
%assign              ISO.Sectors        ISO.Sectors + 1

ISO.Boot.Catalog:

; Add Validation entry, and calculate Checksum for us
ISO.Boot.Validation ISO.Validation      ISO.Catalog.ID, ISO.Boot.Platform.x86

ISO.Boot.Default:
.Bootable       DB              88h
.Media          DB              ISO.Boot.Media.Floppy.1440 ; HardDisk ; NoEmulation
.Load           DW              0000h   ; This could be 7C00h. 0000h defaults it
.System         DB              00h
                DB              0
.Count          DW              1       ; You can use this to load MORE!
.LBA            DD              0       ; Start sector to load
.Reserved       TIMES           (1Fh-0Ch+1) DB 00h

ISO.Boot.Catalog.Padding EQU    ISO.Sector.Size - ($ - ISO.Boot.Catalog)
                TIMES           ISO.Boot.Catalog.Padding DB 00h
