;
; Exec/User/User.inc
;

; Now that everything is set up, we can start to generate User-mode Programs.
; Each Program is essentially the same: code that bounces a ball around its
; defined area of the screen. To fully demonstrate the features of Protected
; Mode, the following is used:
; 1) The Code runs in User Mode (Ring 3);
; 2) Every Program has its own LDT;
; 3) The CPU's Task Switching mechanism using TSSs is used;
; 4) Every Program has its own Stacks and Data, but shares the same Code.
; Once all the Tasks have been set up, the Executive merely waits for
; <Ctrl><Alt><Del> to be pressed - and then resets the computer... At least it
; would: that's been left as an exercise for the reader. At the moment, you can
; use the trusty Power button!

%push Exec.Start ; Let's not leave these %defines just lying around...

%define         %$Pos           EBP - 4
%define         %$LDT           EBP - 8

;...............................................................................
%include        "Exec/User/LDT.inc" ; Creates one LDT for a User-mode Program
;...............................................................................
%include        "Exec/User/TSS.inc" ; Creates one TSS for a User-mode Program

Exec.User:
                ENTER           8, 0

                XOR             EDX,EDX         ; Zero high part of EDX
;               MOV             DL, 0           ; Start full left
                MOV             DH, 1           ; Start one row low

.Loop:
                MOV             [%$Pos],EDX     ; Store away new position

                ; Create a new User LDT
                CALL            Exec.User.LDT
                TEST            EAX,EAX
                JZ              .Finished

                ; Create a TSS to run that User Task
                CALL            Exec.User.TSS
                TEST            EAX,EAX
                JZ              .Finished

                MOV             EDX,[%$Pos]              ; Current position
                ADD             DL,Window.Width          ; Next Window
                CMP             DL,VGA.Cols-Window.Width ; Too far?
                JBE             .Loop                    ; Not yet...

                MOV             DL,0            ; Start at left again
                ADD             DH,Window.Height          ; Next row
                CMP             DH,VGA.Rows-Window.Height ; Too far?
                JBE             .Loop                     ; Not yet...

.Finished:
                HLT
; Wait for <Ctrl><Alt><Del>
                JMP             .Finished

%pop
