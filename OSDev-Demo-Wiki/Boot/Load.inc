;
; Boot/Load.inc
;

; This module loads the rest of the system - all the Protected Mode stuff.
; It loads it into RAM way down low, leaving the rest of RAM for the system's
; use - and allowing this boot code to be jettisoned.

; The number of Sectors to load. Note that the resultant value may exceed what
; the BIOS can handle: the typical maximum is "number of sectors per track".
; Different floppy formats use different numbers: 8, 9, 15, 18, and 36 are known.
; Since the lowest value is 8, that's the "safest" value. 1.44MB floppies use 18
Load.Sectors    EQU             Load.Size / BIOS.Disk.Sector.Size

; Upon entry, DL is BIOS drive number, ES:SI is Hard Disk Partition entry (if any)
Boot.Load:
                MOV             AX,Load.Base>>4    ; Segment to load to
;               MOV             ES,AX              ; Not yet! Need BIOS value
                XOR             BX,BX              ; Offset to load to

                TEST            SI,SI              ; Sensible value?
                JZ              .Beginning         ; Not really...

; Assume pointing to MBR Table entry
                MOV             CX,[ES:SI+BIOS.MBR.Entry.StartCylSect]
                MOV             DH,[ES:SI+BIOS.MBR.Entry.StartHead]

; Now check if above assumption is true!
                OR              SI,BIOS.Disk.Sector.Size-1; Round UP to BIOS.Sig
                CMP             [ES:SI-1],WORD BIOS.Sig.Value ; (Well, nearly!)
                JE              .Continue          ; All that hard work panned out!

; Assumption NOT true: no help from BIOS! Just load from first drive sector
.Beginning:
                MOV             CX,00_01h          ; Cyl 0, Sect #1
                MOV             DH,0               ; Head Start
.Continue:

                INC             CX                 ; Next sector
;               MOV             DL,                ; DriveNum provided by BIOS
                MOV             SI,3               ; Try this many times
                MOV             ES,AX              ; Can load buffer segment now!

.Attempt:
                MOV             AH,BIOS.Disk.Fn.Read ; Read sectors
                MOV             AL,Load.Sectors    ; Number sectors
.NumToRead      EQU             $-1                ; GASP! Self-modifying code!
.Read:
                INT             BIOS.Disk.Int      ; BIOS Read
                JNC             .ReadSome          ; Carry set on error
                CMP             AL,1               ; Tried single-sector reads?
                JE              .Reset             ; Yep! Better reset drive
                MOV             AL,1               ; No. Try that then.
                JMP             .Read              ; And retry command
.ReadSome:
                SUB             [CS:.NumToRead],AL ; Read enough?
                JZ              .Done              ; Success!

; Note that the next calculation only works for a few number of sectors.
; If a larger number were required, the "Sectors per Track" would overflow,
; and we'd have to instead increase the Head count and zero the Sector count.
; If it overflowed again, we'd have to increase the Cylinder count and zero the
; Head and Sector counts. All too hard if we don't really need to do it,
; especially since determining what constitutes "overflow" is tricky!
; Finally, the "memory address to load to" calculation would overflow, limiting
; the final result to 64K maximum anyway
                ADD             CL,AL              ; Fewer sectors to read...

; Turn the Number Sectors Read value into a pointer offset:
                CBW
                IMUL            AX,BIOS.Disk.Sector.Size
                ADD             BX,AX              ; Further in buffer...
.Reset:
                MOV             AH,BIOS.Disk.Fn.Reset
                INT             BIOS.Disk.Int      ; BIOS Reset
                DEC             SI                 ; One less try...
                JNS             .Attempt           ; (Leaves Z clear on last error!)
                JMP             $                  ; ***STOP!***
.Done:
