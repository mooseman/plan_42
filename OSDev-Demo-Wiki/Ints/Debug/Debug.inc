;
; Ints/Debug.inc
;

; This module leverages the '386 debug facilities to implement a "debugger".
;
; The original IBM PC only entered this interrupt as a result of an INT 3
; instruction. The '386 added to this with the concept of "Hardware Debug
; Registers". These could be set to make an INT 3 occur on a number of different
; system events: executing an instruction at a particular address (without
; modifying that instruction to make it an INT 3 first); or, particularly
; useful, writing to a particular memory location.
;
; But all that this "debugger" does is show the memory contents of (readable)
; Segments on the screen. This can be VERY useful! Directly after carefully
; crafting a system table in memory, it is handy to add a DEBUG (INT 3) so that
; you can examine the memory, and confirm that everything is as you designed it.
; The alternative could be a triple-fault...
;
; To invoke the debugger in code, merely call the "DEBUG" macro (which does an
; INT 3). Or, at runtime, press the Pause/Break key (by default: get it?!). To
; show that the debugger is working, a Dingle(tm) is active in the top right
; corner.
;
; Once the memory dump is showing, you can Scroll or Page up and down, or choose
; other displayable Segments (Left and Right). For giggles, display the alias
; for the Screen, and scroll to see the changing Dingle(tm). Then scroll further
; to see the changes to the Screen that are showing the Dingle(tm). Inception!
;
; To exit the Debugger, press Esc (by default). It will return and continue
; where it left off. One thing to note: while the Debugger is running, you can
; look at it by... invoking the Debugger. Press Break again. And again. And
; again. You'll have to exit the same number of times you entered. To prove it,
; go look at the Ring0 Stack that's active, and watch it get deeper, and deeper,
; and... You had to do it, didn't you? Well, it's your own fault!

; Here are the colours for the different fields of the display
Debug.Colour.Addr   EQU         Dev.VGA.Grey  ; Colour of Address field
Debug.Colour.Bytes  EQU         Dev.VGA.White ; Colour of Hex Bytes field
Debug.Colour.DWords EQU         Dev.VGA.White ; Colour of Hex DWords field
Debug.Colour.ASCII  EQU         Dev.VGA.Grey  ; Colour of ASCII field
Debug.Colour.Label  EQU         Dev.VGA.White ; Colour of Labels
Debug.Colour.Regs   EQU         Dev.VGA.Grey  ; Colour of Regs field
Debug.Colour.Blank  EQU         Dev.VGA.Grey  ; Colour when there's nothing there

; Here are the scan-codes for the different functions that the debugger can do
Debug.Key.Quit   EQU            Dev.Key.Break | Dev.Key.Esc ; Quit on Esc UP
Debug.Key.Up     EQU            Dev.Key.Up    ; Scroll up (earlier in memory)
Debug.Key.Down   EQU            Dev.Key.Down  ; Scroll down (later in memory)
Debug.Key.PgUp   EQU            Dev.Key.PgUp  ; Lots of .Ups
Debug.Key.PgDn   EQU            Dev.Key.PgDn  ; Lots of .Downs
Debug.Key.Format EQU            Dev.Key.Del   ; Toggle between Bytes and DWords
Debug.Key.Prev   EQU            Dev.Key.Left  ; Previous Segment
Debug.Key.Next   EQU            Dev.Key.Right ; Next Segment

; Screen coordinates to use
Debug.VGA.Top    EQU            (VGA.Rows-Debug.VGA.Height+1)/2
Debug.VGA.Left   EQU            (VGA.Cols-Debug.VGA.Width+1)/2
Debug.VGA.Width  EQU            Debug.Width.Addr+Debug.Width.Byte
Debug.VGA.Height EQU            VGA.Rows-1
Debug.Show.Height EQU           Debug.VGA.Height-Debug.Regs.Rows
Debug.Regs.Rows  EQU            4

; These constants could be hard to change...
Debug.Width.Addr  EQU           4+1+8+3
Debug.Width.Byte  EQU           (2+1)*Debug.BytesPerRow+Debug.BytesPerRow ; Chars/Byte + ASCII
Debug.Width.DWord EQU           (8+2)*Debug.BytesPerRow/4  ; Chars/DWord
Debug.BytesPerRow EQU           16

;-------------------------------------------------------------------------------
Ints.Debug:
                PUSH            0               ; Duplicate Fault's stack
                PUSH            3               ; Duplicate Fault's stack

                PUSHAD
                CALL            Ints.Fault.Frame.Create

                CALL            .Init           ; Initialise everything
                PUSH            EAX             ; Remember TSS to switch back

                CALL            .Regs           ; Display registers

                XOR             AX,AX           ; Starting Descriptor
                MOV             DS,AX
                CALL            .Desc.Next      ; Get first (real) Descriptor to Show
.Refresh:
                INC     BYTE    [ES:(VGA.Cols-1)*2] ; Display Dingle(tm)

                CALL            .Show           ; Display memory
                CALL            .Key            ; Look at keys
                CMP             AL,Debug.Key.Quit ; Finished?
                JNZ             .Refresh        ; Not yet, so re-display
.Finish:
                POP             EBX             ; Get TSS back
                CALL            .Done

                CALL            Ints.Fault.Frame.Destroy
                POPAD
                ADD             ESP,8           ; Ignore Int and "Error" values
                IRETD           ; Return to next instruction
;-------------------------------------------------------------------------------
%include        "Ints/Debug/Init.inc"   ; Initialisation functions
;-------------------------------------------------------------------------------
%include        "Ints/Debug/Regs.inc"   ; Display registers functions
;-------------------------------------------------------------------------------
%include        "Ints/Debug/Show.inc"   ; Display memory functions
;-------------------------------------------------------------------------------
%include        "Ints/Debug/Key.inc"    ; Key functions
