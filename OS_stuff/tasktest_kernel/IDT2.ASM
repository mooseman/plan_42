; idt2.asm  Set up IDT
; Version 3.0, Mar 20, 1998
; Sample code
; by John S. Fine  johnfine@erols.com
; I do not place any restrictions on your use of this source code
; I do not provide any warranty of the correctness of this source code
;_____________________________________________________________________________
;
; This is a run-time initialized version of the idt.asm that is in some of my
; other samples.
;
; A run-time initialized version has the advantage that the idt itself is not
; stored in the image file.
;
; The link-time initialized version has the advantage that modules can
; override a default vector at link time with a patch_vec macro.  The main
; module doesn't need to replace the default at run time nor to call a
; module init routine to do it.  This makes it easier to share source code
; amoung several sample projects.
;
; The link-time initialized version also allows an IDT in ROM, for BIOS or
; embedded systems projects.
;
; This run time initialized version may be easier to understand, so I wrote
; both idt.asm and this idt2.asm.
;_____________________________________________________________________________

  %include  "gdt.inc"		;Descriptor definitions

  GLOBAL  init_idt		;Initialize the IDT
  GLOBAL  idt			;The IDT itself

  extern  set_vector		;Setup one vector in the IDT

  extern  UNMAPPED_PAGE4IDT	;The system definition must reserve a range
				;of 256 unmapped addresses for marking
				;unsupported interrupt vectors.

  extern  page_fault		;Page fault handler (stack dump in most of my
				;sample source packages)

SEGMENT BSS USE32 ALIGN=16

idt	resb	8*256		;IDT

SEGMENT INIT USE32 ALIGN=2

loadidt	dw	8*256-1		;Length of idt
	dd	idt    		;Linear address of idt

init_idt:
;
;Purpose:
;	Initialize the IDT
;Input:
;	none
;Output:
;	none
;Clobbers:
;	eax, ecx
;_____________________________________________________________________________

; Point all interrupts at unmapped memory

	mov	eax, UNMAPPED_PAGE4IDT
	mov	cx, D_PRESENT+D_INT+D_DPL3
.1:	call	set_vector
	inc	eax
	inc	cl
	jnz	.1

; Point page fault at stack dump routine

	mov	eax, page_fault
	mov	cx, 0Eh + D_INT + D_PRESENT
	call	set_vector
        lidt    [loadidt]		;Load IDT

	ret				;Return
