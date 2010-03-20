; tasktest.asm  Multi-tasking test
; Version 1.0, Mar 25, 1998
; Sample code
; by John S. Fine  johnfine@erols.com
; I do not place any restrictions on your use of this source code
; I do not provide any warranty of the correctness of this source code
;_____________________________________________________________________________
;
;  This is the main module of an example in task scheduling.
;  Please see the accompanying read.me file for detailed documentation.
;
;  This module contains both the code that is specific to the 8254 chip and
;  the code for the sample problem used for the demonstration.
;_____________________________________________________________________________

  GLOBAL  diag_events
  GLOBAL  tss_dsc
  GLOBAL  tss_sel

  %include "tmr.inc"		;8254 timer register definitions
  %include "gdt.inc"		;Descriptor definitions
  %include "8259.inc"		;8259 interrupt controller definitions
  %include "task_t.inc"		;Task data structure
  %include "tss.inc"		;TSS

; External routines:
  extern  init_8259		;Initialize the 8259
  extern  init_idt		;Initialize the IDT
  extern  init_memory		;Initialize memory pools
  extern  anti_sift_new		;Add one event to pri_heap
  extern  set_vector		;Setup one vector in IDT
  extern  timer_service		;Task switcher
  extern  new_task		;Task creator
  extern  dispatch_task		;Start first task
  extern  _screen_counter	;Code executed by the tasks

; External data
  extern  pri_heap		;Priority heap
  extern  B8000			;The display is at 0xB8000 in most systems, I
				;use a link time constant to support other
				;values.  (A product, rather than a sample,
				;should have a run time variable for this).
  extern  tasks			;Array of task structures

  SEGMENT START USE32
  SEGMENT STACK USE32 align=16
  SEGMENT BSS   USE32 align=16
  SEGMENT DATA  USE32 align=16

SEGMENT STACK
	resb	256
stacktop:

SEGMENT DATA
diag_events  dd 0			;Performance diagnostic

SEGMENT START
start:	lgdt	[gdt]			;Switch to our own gdt
	push	ds			;Switch to our own stack
	pop	ss
	mov	esp, stacktop

	call	init_idt		;Initialize standard modules
	call	init_8259
	call	init_memory

;Build our set of tasks:  This builds 200 tasks, each of which increments an on
; screen counter.  They are arranged on screen in twenty rows of ten.  All the
; tasks in one column have the same share (small share to the left, large share
; to the right).  All the tasks in one row have the same defer (small defer at
; the bottom, large defer at the top).

	mov	ebx, B8000		;Address of screen
	mov	ecx, 20			;Number of rows and "defer"

.10:	mov	ebp, 160		;Offset on line and "share"

.20:	call	new_task		;Make a new task
	lea	eax, [ebx + ebp]	;Screen location for its counter

	mov dword [edi+EVT.eip], _screen_counter    ;Code to execute
	mov  byte [edi+EVT.eflags+1], 2		    ;Enable its interrupts
	mov	  [edi+EVT.eax], eax		    ;Screen location
	mov	  [edi+EVT_defer], ecx		    ;"defer"
	push	ecx			
	imul	ecx, ebp			    ;Quantum = defer * share
	mov	  [edi+EVT_quantum], ecx	    ;Quantum

	push	ebx
	mov	ebx, edi		;Task structure address
	call	anti_sift_new		;Insert new event in priority heap
	pop	ebx
	pop	ecx

	sub	ebp, 16			;Next offset and "share"
	jnz	.20
	movzx	eax, word [0x44A]	;Columns on screen
	lea	ebx, [ebx+2*eax]	;Advance to next row
	loop	.10			;Next row and "defer"

	mov	al, TMR_SC0 + TMR_both + TMR_MD0  ;Set channel 0 to mode 0
	out	TMR_CTRL, al

	mov	al, 0xFE		;Disable all IRQ's except timer
	out	M_IMR, al

	mov	eax, timer_service	;Point vector at timer service rotine
	mov	cx, M_VEC + D_INT + D_PRESENT
	call	set_vector

	mov	ax, tss_sel
	ltr	ax
	mov byte [tss_dsc+2], EVT_tss	;Low byte of address, for pointing
					;descriptor at tasks.

	mov byte [tss_dsc+5], (D_TSS+D_PRESENT)>>8  ;Pretend it isn't busy
	jmp	dispatch_task		;Start first task

	alignb 16
tss0	resb	tss_size

SEGMENT DATA
	alignb 16
gdt	    start_gdt
flat_code   desc  0,		    0xFFBFF,    D_CODE+D_READ+D_BIG+D_BIG_LIM
flat_data   desc  0,		    0xFFFFF,    D_DATA+D_WRITE+D_BIG+D_BIG_LIM
tss_dsc:
tss_sel     desc  tss0,		    tss_size-1, D_TSS
	    end_gdt
