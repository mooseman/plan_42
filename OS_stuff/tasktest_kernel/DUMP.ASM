; dump.asm  Stack dump in case of failure
; Version 3.0, Mar 20, 1998
; Sample code
; by John S. Fine  johnfine@erols.com
; I do not place any restrictions on your use of this source code
; I do not provide any warranty of the correctness of this source code
;_____________________________________________________________________________
;
;  The page_fault routine in this module is designed to be hooked as the int
;  0xE handler in test versions of pmode projects.  No recoverable page faults
;  are supported.  Here I assume all page faults are unrecoverable errors (and
;  elsewhere, I force most unrecoverable errors to be page faults).
;_____________________________________________________________________________

  GLOBAL  page_fault

  extern  B8000		;The display is at 0xB8000 in most systems, I use a
			;link time constant to support other values.  (A
			;product, rather than a sample, should have a
			;run time variable for this).

  extern  FLAT_CODE	;Code selector

; The following constant is defined in the .lnk file.  It combines the
; important bits from the constants FLAT_CODE and UNMAPPED_PAGE4IDT
;_____________________________________________________________________________
  extern  UNMAPPED_VAL4IDT

SEGMENT CODE USE32

page_fault:
;
;  Every page fault comes here.  Every unsupported interrupt goes to unmapped
;  memory and triggers a page fault.  First we figure out which happened,
;  then we push registers and dump the stack.
;
;  The stack dump figures out whether the failure occurred in pmode or v86
;  mode and whether there is an error code on the stack.  It labels the
;  locations of registers on the stack accordingly.
;
;  After the main stack dump it displays a column headed by the linear
;  address of the interrupted code, and containing dwords fetched from
;  that address.
;
;  Last it gives a column of either the V86 stack or more of the pmode
;  stack.
;_____________________________________________________________________________

;  The stack dump should be the absolute end of the line.  If we somehow get
;  here again, something is very wrong.  The first instruction changes itsef
;  to a JMP $ so that it will hang on the second time around.
;_____________________________________________________________________________
	mov word [ss:$], 0xFEEB
	cld

	cmp dword [esp+5], UNMAPPED_VAL4IDT	;Unsupported interrupt?
	jne	true_page_fault			;No
	mov	[esp+12], edi			;Yes: rearrange stack
	mov	[esp+8], esi
	xchg	[esp+4], ebp
	push	ebx
	push	edx
	push	ecx
	push	eax
	push	ds
	push	es
	and	ebp, 0FFh		;Number of unsupported interrupt
	push	ebp
	jmp	stack_dump		;JMP to stack dump

true_page_fault:

	pusha				;Save registers
	push	ds
	push	es
	push dword 0xE			;Interrupt number

DBLN	equ	21		;Number of lines to display

stack_dump:
	push	ss		;Setup ds and es
	pop	ds
	push	ss
	pop	es
	mov	edi, B8000	;Point at display
	mov	esi, reg_list	;List of register names
	mov	ebp, esp	;Point at stack contents
	mov	bl, DBLN	;Lines to display
	mov	bh, 0
.1:	mov	eax, ebp	;Display address
	call	hex8
	mov	eax, [ebp]	;Display contents
	call	hex8
	add	ebp, 4		;Next address
	mov	cl, [0x44A]	;Screen width
	sub	cl, 18		;Minus 18 characters used
	cmp byte [esi], al	;Past end of reg_list?
	jz	.3		;Yes

	cmp dword [esi], 'eip'	;Is it eip?
	jnz	.2		;No
	mov	edx, [ebp+4]	;Flags
	and	edx, 0xFFFE802A
	cmp	edx, 0x20002	;V86 mode?
	je	.15		;possibly
	cmp	edx, 2		;Really flags?
	jne	.3		;No
	cmp word [ebp], FLAT_CODE ;Segment correct?
	jne	.3		;No
	cmp word [ebp-2], 0xFF80 ;eip correct?
	jne	.3		;No
	movzx	edx, bl		;words left
	lea	edx, [ebp+edx*4]
	push	edx
	mov	edx, [ebp-4]	;Remember eip
	mov byte [vregs], 0
	jmp	.19
.15:	cmp word [ebp-2], 0	;V86 eip?
	jnz	.3		;No
	cmp word [ebp+10], 0	;V86 esp?
	jnz	.3		;No
	movzx	edx, word [ebp+12]
	shl	edx, 4
	add	edx, [ebp+8]
	push	edx
	movzx	edx, word [ebp]	;cs
	shl	edx, 4
	add	edx, [ebp-4]
.19:	inc	bh
.2:	lodsb			;Display register name
	stosw
	test	al, al		;  null terminated
	loopnz	.2
.3:	rep stosw		;Clear to start of next line
	dec	bl		;Count lines
	jnz near .1

	dec	bh
	js	.99
	mov	edi, B8000+50
	call	hexlist
	pop	edx
	mov	edi, B8000+68
	call	hexlist
.99:	jmp short $		;Hang

hexlist:
	mov	eax, edx
	call	hex8	
	mov	cl, [0x44A]	;Screen width
	lea	edi, [edi+ecx*2]
	mov	bl, DBLN-2
.10:	mov	cl, [0x44A]	;Screen width
	lea	edi, [edi+ecx*2-18]
	mov	eax, [edx]
	add	edx, 4
	call	hex8
	dec	bl
	jnz	.10
	ret

hex8:
;Purpose:
;	Converts value of eax in hex into display buffer pointed to by edi
;Inputs:
;	eax value
;	edi display pointer
;Outputs:
;	edi advanced
;	ecx zero
;	ax  0x700
;_____________________________________________________________________________

	mov	ecx, 8		;Count eight digits
.1:	rol	eax, 4		;Next digit
	push	eax		;Save
	and	al, 0xF		;Convert nibble to hex ascii
	cmp	al, 10
	sbb	al, 0x69
	das
	mov	ah, 7		;Default screen attribute
	stosw			;Store it and advance edi
	pop	eax		;Restore
	loop	.1		;Eight digits
	mov	ax, 0x700	;Invisible character, default attribute
	stosw			;Store it and advance edi
	ret			;Return

SEGMENT DATA
reg_list db	"Vector",0	;Names for items in the stack dump
	db	"es",0
	db	"ds",0
	db	"eax",0
	db	"ecx",0
	db	"edx",0
	db	"ebx",0
	db	" ",0		;Useless item
	db	"ebp",0
	db	"esi",0
	db	"edi",0
	db	"eip",0
	db	"cs",0
	db	"flags",0
vregs:
	db	"esp",0
	db	"ss",0
	db	"es",0
	db	"ds",0
	db	"fs",0
	db	"gs",0
	db	0		;Extra null terminates list

