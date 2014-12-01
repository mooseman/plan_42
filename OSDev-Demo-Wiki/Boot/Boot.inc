;
; Boot/Boot.inc
;

; This is the BIOS Boot entry point. It runs in Real Mode, and assumes the
; standard BIOS Boot Specification 1.01.
;
; It performs the following actions:
; 1) Loads the rest of the code into RAM;
; 2) Gets some information from the soon-to-be-unusable BIOS;
; 3) Sets up the PC for Protected Mode;
; 4) JMPs to the freshly-loaded Protected Mode code
; After that, the memory used to hold this code is avaiable to the system.
;
; Note that one of the goals of the Boot sector is to jettison itself:
; 7C00h is such an awkward location, so ideally after doing all the Real Mode
; initialisation, we can JMP to Protected Mode and simply leave this code behind.

; To save space, I take advantage of some HORRIBLE things you can do with
; assembly language: PUSHing values to fake CALLs, or worse, SELF-MODIFYING CODE!
; In Protected Mode, this last is very difficult to organise - and unreliable
; with later processors that have on-board caches - so not worth it.
; In Real Mode, this can be very useful as a register- and code-saving device!

; To save space, I've done away with byte-wasting CALLs and RETs for "modular"
; programming. Not that modules aren't in and of themselves useful - indeed,
; they're still modularised within their own little files - but the result is
; simply a stream of bytes to execute. Where I have defined subroutines to be
; CALLed, the stream of code has to either JMP over the subroutine's RET, or
; a CALL is faked by PUSHing the desired continuation address onto the stack.

;-------------------------------------------------------------------------------
                SEGMENT         Real  VSTART=0  ALIGN=1

                USE16                      ; Real Mode? 16 bits!

; At this point, DL is probably the drive number of the Boot Drive.
; ES:SI is possibly a pointer to the MBR Partition table entry (See Load.inc).
; In any case: don't clobber DL, ES or SI before Load.inc!

%include        "Boot/Entry.inc" ; Entry code to initialise important registers
;-------------------------------------------------------------------------------
%include        "Boot/CPU.inc"  ; Check CPU is (at least) a '386
;-------------------------------------------------------------------------------
%include        "Boot/Load.inc" ; Load Protected Mode code
;-------------------------------------------------------------------------------
%include        "Boot/IDT.inc"  ; Move IDT to final location
;-------------------------------------------------------------------------------
%include        "Boot/RAM.inc"  ; Get RAM size
;-------------------------------------------------------------------------------
%include        "Boot/A20.inc"  ; Enable A20 gate
;-------------------------------------------------------------------------------
%include        "Boot/Key.inc"  ; Speed up keyboard
;-------------------------------------------------------------------------------
%include        "Boot/VGA.inc"  ; Switch Video mode
;-------------------------------------------------------------------------------
%include        "Boot/PM.inc"   ; Switch to Protected Mode
;-------------------------------------------------------------------------------

; At this point, we still must be inside the first 512-byte Boot sector.
; We need to confirm to the BIOS that this sector is a valid Boot sector by
; having BIOS.Sig.Value at the correct place.
; Note that if the previous code was too large, the following Pad calculation
; will underflow and generate an error.

Real.Padding    EQU             BIOS.Sig.Pos - ($ - $$)
TIMES           Real.Padding    DB 00h

                DW              BIOS.Sig.Value
