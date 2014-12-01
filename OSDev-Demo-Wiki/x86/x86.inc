;
; x86/x86.inc
;

; The following are definitions for the 80386 and later CPUs.
;
; Note that they're defined by Intel as part of the 'DNA' of the chip: almost
; literally, if you think of the silicon as defining the CPU's operation.

;-------------------------------------------------------------------------------
%include        "x86/EFlags.inc"        ; Definitions for EFlags register
;-------------------------------------------------------------------------------
%include        "x86/Seg.inc"           ; Definitions for Segment register
;-------------------------------------------------------------------------------
%include        "x86/CR0.inc"           ; Definitions for CR0 register
;-------------------------------------------------------------------------------
%include        "x86/Desc.inc"          ; Definitions for Descriptors
;-------------------------------------------------------------------------------
%include        "x86/TSS.inc"           ; Definitions for Task State Segment
;-------------------------------------------------------------------------------
%include        "x86/IVT.inc"           ; Definitions for Interrupt Vector Table
