; tasks_h.asm  Hardware multi-tasking routines
; Version 1.0, Mar 25, 1998
; Sample code
; by John S. Fine  johnfine@erols.com
; I do not place any restrictions on your use of this source code
; I do not provide any warranty of the correctness of this source code
;_____________________________________________________________________________
;
;  A single descriptor in the GDT is used for all TSS's.  In order to make
;  that trick work:
;
;  1)  I overwrite the address portion of the descriptor each time the
;      current task is changed.  To simplify that step, I assume that all the
;      TSS's lie in the task structures in the "tasks" array, and that the
;      task structures are aligned on 256 byte boundaries.  This allows me to
;      update only the middle word of the address.
;
;  2)  The descriptor must be marked nonBusy before JMPing to it.  In the
;      current version, that is done exactly once (after the LTR in the
;      startup code).  The JMP does the following in sequence:
;         I)   Check that the new descriptor is not busy.
;         II)  Mark the new descriptor as busy.
;         III) Mark the old descriptor as not busy.
;      Since the old descriptor IS the new descriptor, it ends up not busy
;      even though it is the current descriptor.
;
;      If this code is used in an environment in which some task switches
;      occur outside its control (such as interrupts to task gates), it may
;      be necessary to uncomment the step of marking the descriptor not busy
;      before the JMP.
;
;  If you don't like the trick of sharing a descriptor at all, I suggest
;  that you add a word in the EVT_ section of the task data structure to
;  hold the selector of task's TSS.  You can the switch to the task with
;  something like "JMP far [esi+EVT_selector-4]".  Since it is a selector
;  for a task, the CPU will ignore the "offset" (the dword before the
;  selector).
;_____________________________________________________________________________
;
;  Instead of a priority value, each task's execution is controled by two
;  parameters:  "quantum" and "defer".
;
;  Each time a task gets a turn at the CPU it gets quantum "microticks".  A
;  microtick is about 839 nanoseconds.  In most systems you will want the
;  quantum for typical tasks to be several thousand (one millisecond is about
;  1192 microticks).
;
;  The higher defer is, the less often a task gets a turn.
;
;  A task's "share" of total execution is equal to quantum/defer.
;
;  A task's fraction of total execution time is its share divided by the total
;  of the shares of all active tasks.
;
;  Note that quantum and defer are integers, but share can be any value.  In
;  fact share is never actually calculated by the scheduler.  The scheduler
;  works strictly with quantum and defer.  Share just happens as a result.
;_____________________________________________________________________________
;
;  Understanding the behavior of very small quantum values (ignore this if
;  you plan to use reasonable quantum values):
;
;  Part of the task switch time occurs outside of quantums (after the end of
;  one quantum and before the beginning of the next).  On a 50 Mhz 486,
;  running 200 tasks, I measured that at 14.6 microseconds (with fewer tasks
;  it would be just a little faster).
;
;  Part of the task switch time occurs inside the quantum of the next task.
;  On that 50 Mhz 486, I measured that at 10.3 microseconds.  If the
;  quantum had been less than 13 microticks the task wouldn't have ever
;  executed any instructions.
;
;  Depending on the phase of the 8254 as each task is dispatched, there is
;  between zero to one extra microtick added to the quantum.  On one
;  overclocked 6x86 that I tested, that fractional extra microtick
;  averaged more than the time for the portion of task switching within
;  the quantum, so for example, 10 turns of 16 microticks each was
;  slightly more CPU time than one turn of 160 microticks.  On most CPUs
;  it is much the other way.  (Of course, 10 turns of 16 microticks
;  always takes more actual time than one turn of 160 microticks,
;  because some of the task switch time is between quantums).
;_____________________________________________________________________________

MAX_TASKS  EQU  1024		;Maximum number of tasks

  GLOBAL  timer_service		;Main entry point for task switching
				; (Startup code must hook this to IRQ 0)
  GLOBAL  dispatch_task		;Alternate entry point for task switching
  GLOBAL  new_task		;Routine to create a new task
  GLOBAL  tasks			;Array of task data structures

  %include  "task_t.inc"
  %include  "tmr.inc"
  %include  "8259.inc"
  %include  "gdt.inc"

  SEGMENT EMPTY ALIGN=4096
  SEGMENT CODE  USE32
  SEGMENT DATA

  extern  sift_heap             ;Adjust priority heap
  extern  alloc_lin_page	;Allocate a page

  extern  diag_events		;Diagnostic counter
  extern  pri_heap		;Priority heap of active tasks
  extern  tss_sel		;tss selector
  extern  tss_dsc		;tss descriptor

SEGMENT EMPTY
tmp	resb	4096*2		;Temporary pages

tasks	resb	EVT_size*MAX_TASKS

SEGMENT DATA
task_space dd	tasks

SEGMENT CODE

new_task:
;
;Purpose:
;	Create a new task
;Input:
;	none
;Output:
;	edi = address of task structure
;Clobbers:
;	eax, edx
;
;  Each task is created with a private page directory (copied from the current
;  page directory),  a private page table (mapping FF400000 - FF7FFFFF) and
;  a private stack (FF7FF000 - FF7FFFFF).
;
;  In the task data structure, the cr3, esp, ss, cs, ds and es registers are
;  all initialized to appropriate values.  All other registers and other
;  fields in the task data structure are cleared.
;
;  The caller should supply at least EVT.eip, EVT_quantum, and EVT_defer.
;  Except on system startup, the caller must supply EVT_time (usually by
;  adding or subtracting a constant from the EVT_time of the task that caused
;  the new task to start).
;
;  The caller should also insert the new task in some queue.
;_____________________________________________________________________________

	push	ebx
	push	ecx
	push	esi
	mov	ebx, 2			;Use memory pool 2

	mov	edi, [task_space]	;Memory already allocated for TSS's
	lea	eax, [edi+EVT_size]	;Advance past new one
	mov	[task_space], eax
	and	eax, 0xFFFFF000		;Down to page boundary
	cmp	edi, eax		;Already allocated this page?
	ja	.10			;Yes
	call	alloc_lin_page		;No: do it now
.10:	push	edi			;Save structure address
	xor	eax, eax		;Clear it
	mov	ecx, EVT_size/4
	rep stosd
	pop	edi			;Restore structure address
	push	edi			;Save structure address
	
	mov	eax, tmp+4096		;Allocate a page for stack
	call	alloc_lin_page

	mov	eax, tmp		;Allocate a private page table
	call	alloc_lin_page
	xchg	edi, eax		;edi = tmp
	xor	eax, eax
	mov	ecx, 4096/4-1		;Clear most of it
	rep stosd
	mov	eax, [edx+4]		;Get mapping for stack
	stosd				;Map in in private page table

	mov	eax, tmp+4096		;Allocate a page directory
	call	alloc_lin_page
	xchg	edi, eax		;edi = tmp+4096
	mov	esi, -4096		;Current page directory
	mov	ecx, 4096/4-1		;Copy most of it
	rep movsd
	mov	eax, [edx-4]		;Get page table mapping
	mov	[edi-8], eax		;Map it as third to last page table
	mov	eax, [edx]		;Get page directory mapping
	stosd				;Self map it
	pop	edi			;Restore structure address
	mov	al, 0			;Make true physical address

	mov	[edi+EVT.cr3], eax	;Set cr3 value
	mov	[edi+EVT.ss], ss	;Set up its stack
	mov dword [edi+EVT.esp], 0xFF800000
	mov	[edi+EVT.ds], ds	;Set segment registers
	mov	[edi+EVT.es], ds
	mov	[edi+EVT.cs], cs

	pop	esi
	pop	ecx
	pop	ebx
	ret

timer_service:
;
;  Get here on each timer interrupt
;
;  For simplicity I will assume this whole routine runs with interrupts off.
;_____________________________________________________________________________

	pusha				;Save registers
	push	ds
	push	ss			;Set up ds (note ss is always valid)
	pop	ds

	mov	al, EOI			;Tell 8259 that interrupt is done
	out	M_PIC, al

	inc dword [diag_events]		;Diagnostic
	mov	esi, [pri_heap]		;Get current task
	mov	eax, [esi+EVT_defer]	;Get its defer value
	add	[esi+EVT_time], eax	;Advance its time value
	call	sift_heap		;Update priority heap
dispatch_task:
	mov	esi, [pri_heap]		;Get next task

	mov	al, [esi+EVT_quantum]	;Program 8254 for next quantum
	out	TMR_CNT0, al
	mov	al, [esi+EVT_quantum+1]
	out	TMR_CNT0, al

	lea	eax, [esi+EVT_tss]	;Get its tss address
	shr	eax, 8			;Just the interesting bits
	mov	[tss_dsc+3], ax		;Change tss descriptor in gdt
;;;	mov byte [tss_dsc+5], (D_TSS+D_PRESENT)>>8  ;Pretend it isn't busy
	jmp	tss_sel:0		;Switch tasks
;Switching tasks saves context in the old tss.  When some other task switches
;back to us, it will continue from after the above JMP.
	pop	ds			;Restore registers
	popa
	iret				;Exit interrupt

