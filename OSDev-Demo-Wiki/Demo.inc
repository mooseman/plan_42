;
; Demo.inc
;

; These are definitions for the rest of the program. You can experiment by
; changing them. Some expect to be changed (these first few), while others
; will have far-reaching impacts.
;

; This %define changes things to invoke the BIOS to show more text:
;%define         BIOS.VGA.90x60 ; Allow use of 90x60 VGA (not universal!)

; Change these to (almost) whatever you'd like. If you're too extreme, you'll
; get assemble-time errors. Others you won't know until you try...
; For example, Timer.Tick is limited on virtualisation programs like VMware,
; so that even programming ludicrous values will result in staid performance.
; However, on real hardware there isn't that sort of control. Go on, set the
; Timer to 1,000,000 times a second - I dare you!
Timer.Tick      EQU             1000    ; Number of Timer Interrupts a second
Window.Width    EQU             8       ; Width of each Task's Window
Window.Height   EQU             6       ; Height of each Task's Window

User.Ball       EQU             1       ; Try 3: it is an OEM 'Heart' symbol
User.BallColour EQU             Dev.VGA.BlueBack  | Dev.VGA.White
User.FrameColour EQU            Dev.VGA.GreenBack | Dev.VGA.White
User.Delay      EQU             0FFFFh  ; How long to 'idle'the Bounce animation

; A useful pseudo-instruction to invoke the built-in Debugger - after it's set up!
%define         DEBUG           DB 0CCh ; INT 3 - NASM assembles this as 0CD 03h

; Another macro for debugging. Enable Single Step debugging - after it's set up!
%macro          TRACE           0
                PUSHFD
                OR      WORD    [ESP],x86.EFlags.TF
                POPFD
%endmacro

; The following is the memory map for this Demonstration. Note that some of the
; areas are left over from the BIOS - I'm simply working around those.
IVT.Base        EQU             x86.IVT.Base
IVT.Size        EQU             x86.IVT.Size
BDA.Base        EQU             BIOS.BDA.Base
BDA.Size        EQU             BIOS.BDA.Size
Data.Base       EQU             BDA.Base + BDA.Size
Data.Size       EQU             IDT.Base - Data.Base

IDT.Base        EQU             Code.Base - IDT.Max ; Final address of IDT
IDT.Max         EQU             x86.Ints * x86.Desc_size ; Full size of IDT

Code.Base       EQU             1000h

Load.Base       EQU             IDT.Start       ; Load from start of defined IDT
Load.Max        EQU             8 * BIOS.Disk.Sector.Size ; See Boot/Load.inc for why

; This will be the first-available RAM after everything else is initialised
Demo.First      EQU             Load.Base + Load.Size + GDT.Size + Exec.Stack.Size

Exec.Stack.Top  EQU             Demo.First      ; This is the TOP of an Expand-Down stack
Exec.Stack.Size EQU             1000h

; The PICs need to be re-based to get them away from the Intel-reserved interrupts
PIC.A.Base      EQU             20h
PIC.B.Base      EQU             28h

; See Ints/Timer.inc for a discussion on this (significant!) definition
System.TSS      EQU             x86.Desc.Mem.Gran.Avail

; This is the Descriptor for the Alias for the Descriptor Table that holds it
DT.Alias        EQU             x86.Desc_size

; The following structure is held in the first (NULL) GDT entry. It holds the
; information about the current state of the GDT: how big it is, and which
; entries have been returned.
; It is designed to enable the the following instruction to be used:
;         LGDT  [GDT.Alloc.Pseudo]
; This is accomplished by:
; 1) Storing the linear base address of the GDT in .Pseudo.Base
; 2) Storing the current GDT size in .Pseudo.Size
; 3) As entries are freed, adding them to the .Free list (this uses the entries
;    themselves to store the next Free one, or 0 to end the list).
                STRUC           GDT.Alloc
.Free           RESW            1       ; Index to first Free Descriptor, or 0
.Pseudo         EQU             $       ; You can do LGDT [GDT.Alloc.Pseudo]
.Pseudo.Limit   EQU             $
.Limit          RESW            1       ; Last allocated Descriptor
.Pseudo.Base    RESD            1       ; GDT.Base, as part of a pseudo-descriptor
                ENDSTRUC

; Like the GDT, the following structure is held in the first LDT entry - even
; though this is a valid entry according to Intel's definition.
; But by putting it there, and setting it up so that the important fields
; overlap the same fields in GDT.Alloc, the same code can be used to
; allocate Descriptors in either table.
; The "important fields" are simply .Free and .Limit
                STRUC           LDT.Alloc
.Free           RESW            1       ; Index to first Free Descriptor or 0
.Limit          RESW            1       ; Last (ever) allocated Descriptor
                RESB            1       ; Reserved
.Type           RESB            1       ; Make sure any attempt at access fails!
.GDT            RESW            1       ; This is the LDT's entry in the GDT
                ENDSTRUC

; As stated above, the two ?DT.Alloc structures have common elements:
                STRUC           DT.Alloc
.Free           RESW            1       ; Index to first Free Descriptor, or 0
.Limit          RESW            1       ; Last allocated Descriptor
                ENDSTRUC
