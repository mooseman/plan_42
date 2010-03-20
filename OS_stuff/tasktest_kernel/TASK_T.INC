; task_t.inc   structure for building tasks
; Version 1.0 March 5, 1998
; Sample code
; by John S. Fine  johnfine@erols.com
; I do not place any restrictions on your use of this source code
; I do not provide any warranty of the correctness of this source code
;_____________________________________________________________________________
;
; This is a combination of the "event" structure and the task segment.
; One of these is required for each task.  It starts with the task data that
; used only by the "OS" and not by the hardware task switching.  This is
; followed by a standard TSS.
;
; Because I am using pri_heap from another source sample, the first to fields
; of this structure must match the first to fields from event.inc
;
; Because of an optimization in task_h.asm, this whole structure must be a
; multiple of 256 bytes.
;
;_____________________________________________________________________________

	struc	EVT
EVT_time equ $
   resd 1	;Time at which this event should next occur
EVT_heap equ $
   resd 1	;Backpointer to this event's pointer in the heap
EVT_quantum equ $
 resd 1	;Quantum
EVT_defer equ $
  resd 1	;Quantum / share
  alignb 16
EVT_tss equ $
.back	resw	2
.esp0	resd	1
.ss0	resw	2
.esp1	resd	1
.ss1	resw	2
.esp2	resd	1
.ss2	resw	2
.cr3	resd	1
.eip	resd	1
.eflags	resd	1
.eax	resd	1
.ecx	resd	1
.edx	resd	1
.ebx	resd	1
.esp	resd	1
.ebp	resd	1
.esi	resd	1
.edi	resd	1
.es	resw	2
.cs	resw	2
.ss	resw	2
.ds	resw	2
.fs	resw	2
.gs	resw	2
.ldt	resw	2
.trap	resw	1
.io	resw	1
	alignb	256
	endstruc

