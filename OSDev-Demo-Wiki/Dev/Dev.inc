;
; Dev/Dev.inc
;

; Different devices have different definitions: for their registers, their
; assumed structures, and for their access protocols.

;-------------------------------------------------------------------------------
%include        "Dev/A20.inc"   ; Definitions for the A20 Gate controller
;-------------------------------------------------------------------------------
%include        "Dev/PIC.inc"   ; Definitions for the Priority Interrupt Controllers
;-------------------------------------------------------------------------------
%include        "Dev/Timer.inc" ; Definitions for the Programmable Interval Timer
;-------------------------------------------------------------------------------
%include        "Dev/Key.inc"   ; Definitions for the Keyboard
;-------------------------------------------------------------------------------
%include        "Dev/VGA.inc"   ; Definitions for the VGA video controller
