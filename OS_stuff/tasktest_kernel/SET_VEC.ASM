; set_vec.asm  Setup one vector in the IDT
; Version 3.0, Mar 20, 1998
; Sample code
; by John S. Fine  johnfine@erols.com
; I do not place any restrictions on your use of this source code
; I do not provide any warranty of the correctness of this source code
;_____________________________________________________________________________

  GLOBAL  set_vector		;Setup one vector in IDT

  extern  idt			;The IDT itself

SEGMENT CODE USE32

set_vector:
;
;Purpose:
;	Setup one vector in IDT
;Input:
;	eax:  address of service routine
;	cl:   interrupt number
;	ch:   major control byte (see gdt.inc)
;             usually (D_INT + D_PRESENT)>>8
;             or      (D_TRAP + D_PRESENT)>>8
;Output:
;	none
;Clobbers:
;	none
;
; Build a gate descriptor as follows:
;
;  subField             Source       Descriptor bits
;  ------------------   -----------  ---------------
;  Offset[0..15]        eax[0..15]        0..15
;  Selector[0..15]      cs               16..31
;  Minor control bits   0                32..39
;  Major control bits   ch               40..47
;  Offset[16..31]       eax[16..31]      48..63
;_____________________________________________________________________________

	push	edx			;Save
	movzx	edx, cl			;Vector number
	mov	 [idt+edx*8], eax	;Bits 0..15 -> 0..15
	mov	 [idt+4+edx*8], eax	;Bits 16..31 -> 48..63
	mov word [idt+2+edx*8], cs	;Selector -> 16..31
	mov byte [idt+4+edx*8], 0	;zero -> 32..39
	mov	 [idt+5+edx*8], ch	;control -> 40..47
	pop	edx			;Restore
	ret				;Return

