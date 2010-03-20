; 8259.asm  Setup 8259 interrupt controllers
; Version 1.1, Nov 29, 1997
; Sample code
; by John S. Fine  johnfine@erols.com
; I do not place any restrictions on your use of this source code
; I do not provide any warranty of the correctness of this source code
;_____________________________________________________________________________

	%include "8259.inc"

SEGMENT INIT USE32

	GLOBAL	init_8259
init_8259:
;
;Purpose:
;	Reinitializes the interrupt controllers.  The usual reason for doing
;	this is to change the vectors.  The master interrupt controller will
;	vector in the range M_VEC to M_VEC+7.  The slave interrupt controller
;	will vector in the range S_VEC to S_VEC+7.
;Input:
;	none
;Output:
;	none
;Clobbers:
;	al
;_____________________________________________________________________________

	mov	al, ICW1		;Start 8259 initialization
	out	M_PIC, al
	out	S_PIC, al
	mov	al, M_VEC		;Base interrupt vector
	out	M_PIC+1, al
	mov	al, S_VEC
	out	S_PIC+1, al
	mov	al, 1<<2		;Bitmask for cascade on IRQ 2
	out	M_PIC+1, al
	mov	al, 2			;Cascade on IRQ 2
	out	S_PIC+1, al
	mov	al, ICW4		;Finish 8259 initialization
	out	M_PIC+1, al
	out	S_PIC+1, al

	mov	al, 0xFF		;Mask all interrupts
	out	M_IMR, al
	out	S_IMR, al

	ret
