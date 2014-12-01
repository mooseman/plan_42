;
; BIOS/BIOS.inc
;

; The original IBM PC BIOS, and various successors, have all embodied relatively
; arbitrary design decisions that have become industry standards. Of course, not
; every BIOS uses the same definitions, and others had bugs which were fixed in
; later versions, but those arbitrary decisions need to be either utilised or
; accommodated by any system wanting to work on the PC.

BIOS.RAMTop.Int EQU             12h

;-------------------------------------------------------------------------------
%include        "BIOS/A20.inc"          ; Definitions for A20 Gate controller
;-------------------------------------------------------------------------------
%include        "BIOS/RAMMap.inc"       ; Definitions for RAMMap information
;-------------------------------------------------------------------------------
%include        "BIOS/Key.inc"          ; Definitions for Keyboard
;-------------------------------------------------------------------------------
%include        "BIOS/VGA.inc"          ; Definitions for VGA video
;-------------------------------------------------------------------------------
%include        "BIOS/Disk.inc"         ; Definitions for Disk controller
;-------------------------------------------------------------------------------
%include        "BIOS/MBR.inc"          ; Definitions for Master Boot Record
;-------------------------------------------------------------------------------
%include        "BIOS/MemMap.inc"       ; Definitions for boot-time memory map
