;
; Exec/Ints/Ints.inc
;

; This module creates handlers for the critical Faults. It's only once this has
; been done that the system is truly safe from the dreaded Triple Fault!
;
; The technique is "simple" - establish a full runtime environment for critical
; Faults so that they don't rely on "piggybacking" the existing environment: in
; other words, give them each their own Stacks. And TSSs. They're almost a
; Program in their own right!
;
; The decision as to which Faults are considered "critical" is straightforward:
; 1) If you're already in the middle of a Double Fault, you'd better get into a
;    stable environment to avoid the Triple Fault!
; 2) If you get a Stack fault, you can't trust the stack any more! If it was at
;    User level, a Ring0 stack is automatically switched in. But what if you're
;    already at Ring0?
; 3) If a Task Switch is attempted, but the TSS turns out to be faulty, which
;    context are you in? The old or the new TSS? Better have a third one handy!
;
; Of course having a separate context is useful, but it introduces its own
; headaches - one of the big ones being that you'd better not suffer a second
; one of these while the first is still being handled! Oh, and better make sure
; that the Task-switching code doesn't try to switch in one of these waiting
; TSSs - they are, after all, ready to run! (We just hope they never do...)

Exec.Ints:
                CALL            Exec.Ints.Trace ; Install a Single-Step (TRACE) handler
                CALL            Exec.Ints.Debug ; Install the Debugger
                CALL            Exec.Ints.NoSeg ; Install an example NoSegment handler

                CALL            Exec.Ints.Double ; Install a Double Fault handler
                CALL            Exec.Ints.BadStack ; Install a Bad Stack handler
                CALL            Exec.Ints.BadTSS ; Install an Invalid TSS handler

                RET
;...............................................................................
%include        "Exec/Ints/Trace.inc"
;...............................................................................
%include        "Exec/Ints/Debug.inc"
;...............................................................................
%include        "Exec/Ints/NoSeg.inc"
;...............................................................................
%include        "Exec/Ints/Double.inc"
;...............................................................................
%include        "Exec/Ints/BadStack.inc"
;...............................................................................
%include        "Exec/Ints/BadTSS.inc"
;...............................................................................
%include        "Exec/Ints/Fault.inc"
