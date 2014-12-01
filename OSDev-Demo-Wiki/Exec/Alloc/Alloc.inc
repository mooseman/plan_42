;
; Exec/Alloc/Alloc.inc
;

; This module handles allocating various system resources in separate helper
; functions.
;
; One sub-module is called DT.inc - it handles allocating in both GDTs and LDTs,
; but requires some setup first: this is performed in the appropriate sub-module.
; As such, it's only to be used internally: alas, assembler doesn't have a
; "private:" designator!

;-------------------------------------------------------------------------------
%include        "Exec/Alloc/RAM.inc"    ; RAM-related functions
;-------------------------------------------------------------------------------
%include        "Exec/Alloc/DT.inc"     ; Descriptor Table helper functions
;-------------------------------------------------------------------------------
%include        "Exec/Alloc/IDT.inc"    ; IDT-related functions
;-------------------------------------------------------------------------------
%include        "Exec/Alloc/GDT.inc"    ; GDT-related functions
;-------------------------------------------------------------------------------
%include        "Exec/Alloc/LDT.inc"    ; LDT-related functions
;-------------------------------------------------------------------------------
%include        "Exec/Alloc/TSS.inc"    ; TSS-related functions
