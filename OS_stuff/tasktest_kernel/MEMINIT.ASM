; meminit.asm  Relocate self and init free page pools
; Version 1.0, Mar 25, 1998
; Sample code
; by John S. Fine  johnfine@erols.com
; I do not place any restrictions on your use of this source code
; I do not provide any warranty of the correctness of this source code
;_____________________________________________________________________________
;
; This (not yet well commented) module assumes that it is in an image loaded
; by bootp or vload or something similar.
;
; It clears out of the page tables, any pages which are not used by the
; current image, except for physical page zero (reserved for the BIOS) and
; (when required by the BIOS) the last physical page of the first 640K.
;
; It sizes RAM and releases all free memory to the control of mempool.asm
;
; It copies the entire image (including itself, the stack, and the page
; tables) to new pages pulled from pool 2.
;
; As set up by vload or bootp, it assumes that the image is mapped into the
; address space at FF800000 and that the page directory is self mapped at
; FFFFF000 (-4096).  This implies that the image's page table is at FFFFE000
; (-2*4096) and that the zeroth page table is at FFC00000.
;
; It also assumes that the stack is linked as the last part of the image that
; should be initially mapped and that esp is still in the top page of the
; stack.
;_____________________________________________________________________________

  GLOBAL  init_memory

  EXTERN  free_mem_pool
  EXTERN  free_phys_page
  EXTERN  alloc_lin_page

SEGMENT INIT USE32

SEGMENT INIT

init_memory:
	mov	edi, esp		;Unmap our own memory beyond
					;the stack
	add	edi, 4095
	sar	edi, 10
	and	edi, byte ~3
	xor	eax, eax
.0:	stosd
	cmp	edi, -4096
	jb	.0

	add	edi, byte 4		;Clear most of page directory
	mov	ecx, 1024-3
	rep stosd

	mov	edi, 0xFFC00004		;Zeroth page table
	mov	dword [edi-4],0x67	;Map page zero normally
	mov	cl, 640/4-1		;Unmap most of conventional RAM
	rep stosd
	cmp	word [0x413], 640	;Does BIOS report 640K?
	je	.05			;Yes
	mov	dword [edi-4],0x9F067	;No: reserve something for EBDA
.05:	mov	eax, 0xA0067		;Self map video and roms
.07:	stosd
	add	eax, 4096
	cmp	eax, 0x100000		;to end of first MB
	jb	.07
	xor	eax, eax
	mov	ecx, 1024*3/4		;Unmap the rest of 4mb
	rep stosd

	mov	eax, 0xFFC00000		;Zeroth page table
	xor	edi, edi
.1:	mov	ecx, [eax]		;Get entry
	cmp	edi, ecx		;Find largest used page
	jae	.2
	mov	edi, ecx
.2:	shr	ecx, 12			;Page number
	bts	[bitmap], ecx		;Mark it used
	add	eax, byte 4		;Next entry
	je	.3
	cmp	eax, 0xFFC01000		;Ending zeroth table
	jne	.1			;No
	mov	eax, 0xFFFFE000		;Yes: skip to second to last
	jmp short .1

.3:	mov	eax, cr0		;Disable cache
	or	eax, 0x40000000
	mov	cr0, eax


.10:	or	edi, 4095		;Next page
	add	edi, byte 0x64		;As page table entry
	mov	[0xFFC00004], edi	;Map as page one
	mov	eax, cr3		;Flush TLB
	mov	cr3, eax
	mov	eax, [4096]		;First dword of page
	mov	al, [0]			;Avoid ghost problems
	not	eax
	mov	[4096], eax		;Test that location
	mov	ecx, [0]		;Avoid bus hold problems
	cmp	eax, [4096]		;Is it memory?
	je	.10			;Yes:  next

	mov	eax, cr0		;Enable cache
	and	eax, ~0x40000000
	mov	cr0, eax

.20:	sub	edi, 4096		;Previous page
	jc	.30
	mov	ecx, edi
	shr	ecx, 12			;Convert to page number
	bt	[bitmap], ecx		;Page Used?
	jc	.20			;Yes
	mov	eax, edi		;No: free it
	call	free_phys_page
	jmp short .20

.30:	mov	esi, 0xFF800000-4096	;Start of our memory
.35:	add	esi, 4096
.40:	cmp	esi, free_mem_pool
	jne	.50
	add	esi, 3*4096

.50:	mov	ebp, esi
	sar	ebp, 10
	cmp	byte [ebp], 0		;Page allocated?
	je	.35			;No
	mov	eax, 4096		;Page 1
	mov	ebx, 2			;Pool 2
	call	alloc_lin_page
	xchg	eax, edi		;Destination
	mov	ecx, 1024
	rep movsd			;Copy the page
	mov	eax, [0xFFC00004]	;Get its mapping
	test	esi, esi		;Is it the page directory?
	jz	.60			;Yes
	xchg	eax, [ebp]		;Use new mapping
	mov	ebx, cr3
	mov	cr3, ebx
	call	free_phys_page
	jmp short .40
.60:	mov	ebx, eax		;Yes:
	xchg	eax, [edi-4]		;Self map it
	mov	byte [4096], 0x67
	and	dword [0xFFC00004], byte 0
	mov	bl, 0
	mov	cr3, ebx
	jmp	free_phys_page

bitmap	resb	512
