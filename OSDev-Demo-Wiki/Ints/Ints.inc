;
; Ints/Ints.inc
;

;-------------------------------------------------------------------------------
%include        "Ints/IDT.inc"          ; First, define an IDT
;-------------------------------------------------------------------------------
%include        "Ints/Vectors.inc"      ; Vectors for each interupt
;-------------------------------------------------------------------------------
%include        "Ints/Fault.inc"        ; Default Fault handling code
;-------------------------------------------------------------------------------
%include        "Ints/Single.inc"       ; Example single-step handler
;-------------------------------------------------------------------------------
%include        "Ints/Debug/Debug.inc"  ; Example debug handler
;-------------------------------------------------------------------------------
%include        "Ints/NoSeg.inc"        ; Example No Segment handler
;-------------------------------------------------------------------------------
%include        "Ints/User.inc"         ; Demonstrate User System Services
;-------------------------------------------------------------------------------
%include        "Ints/IRQs.inc"         ; Vectors for each IRQ
;-------------------------------------------------------------------------------
%include        "Ints/Timer.inc"        ; Example Timer interrupt handler
;-------------------------------------------------------------------------------
%include        "Ints/Key.inc"          ; Example Keyboard interrupt handler
;-------------------------------------------------------------------------------
%include        "Ints/Util.inc"         ; Utility functions
