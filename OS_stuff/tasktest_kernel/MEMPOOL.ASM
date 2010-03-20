; mempool.asm  Manage three pools of free physical pages
; Version 1.0, Mar 25, 1998
; Sample code
; by John S. Fine  johnfine@erols.com
; I do not place any restrictions on your use of this source code
; I do not provide any warranty of the correctness of this source code
;_____________________________________________________________________________

  GLOBAL  mem_pool_sizes
  GLOBAL  free_mem_pool
  GLOBAL  free_lin_page
  GLOBAL  free_phys_page
  GLOBAL  alloc_lin_page

; Note that the low byte of all physical page addresses passed to or from this
; module (or stored in its free pools) is considered random and must be
; ignored.  Bits 8 to 11 of a physical page address are assumed to be zero.
;_____________________________________________________________________________

SEGMENT	CODE USE32
SEGMENT DATA USE32 ALIGN=16
SEGMENT EMPTY ALIGN=4096

SEGMENT EMPTY

; The three pools each occupy a page of linear address space.
; If a pool is empty, there is no physical memory mapped to its linear page.
; The free pages in each pool are grouped into sets of up to 1024 pages
; (indexed 0 to 1023).  Page zero of each set contains the physical page
; address of all the other pages in the set.  Dword zero within page zero
; contains the physical page address of page zero of the next set.
; The PTE (page table entry) for the pool maps the physical page address of
; page zero of the current set.
;_____________________________________________________________________________

free_mem_pool:
pool	resb	4096*3

SEGMENT DATA
mem_pool_sizes:
count	dd	0,0,0

SEGMENT CODE

free_lin_page:
;
;Purpose:
;	Free the physical page from a specified linear address
;Input:
;	eax = linear address of page to free
;Output:
;	none
;Clobbers:
;	eax,ebx,ecx,edx
;_____________________________________________________________________________

	shr	eax, 10			;Convert to offset of PTE
	mov	eax, [0xFFC00000+eax]	;Get physical page
	;jmp	free_phys_page

free_phys_page:
;
;Purpose:
;	Free a physical page.
;Input:
;	eax = physical address of page to free (low byte ignored).
;Output:
;	none
;Clobbers:
;	eax,ebx,ecx,edx
;_____________________________________________________________________________

	xor	ebx, ebx		;Decide which pool it goes in
	cmp	eax, 0x100000		;In first megabyte?
	jb	.1			;Yes
	inc	ebx			;No
	cmp	eax, 0x1000000		;In first 16 megabyte?
	jb	.1			;Yes
	inc	ebx			;No
.1:	mov	edx, ebx		;Construct address of pool
	shl	edx, 12
	add	edx, pool
	mov	ecx, [count+4*ebx]	;Current size of that pool
	inc	dword [count+4*ebx]	;Store new size
	and	ecx, 1023		;On set boundary?
	jnz	.2			;No
	push	edx			;Yes: Save pool address
	sar	edx, 10			;Convert to address of PTE that
					;maps this pool
	mov	al, 0x63		;Make new page address it into a PTE
	xchg	eax, [edx]		;Map it as page zero of a new set
					; eax = page zero of old set.
	mov	edx, cr3		;Flush TLB
	mov	cr3, edx
	pop	edx			;Restore pool pointer
.2:	mov	[edx+4*ecx], eax	;Store free page
	ret

alloc_lin_page:
;
;Purpose:
;	Allocate a physical page for a linear address
;Input:
;	eax = linear address
;	ebx = pool number
;Output:
;	edx = address of page's PTE
;Clobbers:
;	ecx
;_____________________________________________________________________________

	push	ebx			;Save
	mov	edx, eax
	shr	edx, 10			;Convert to address of PTE that
	add	edx, 0xFFC00000		;  maps this page
.1:	mov	ecx, [count+4*ebx]	;Any pages available in this pool?
	dec	ecx
	jns	.2			;Yes
	dec	ebx			;No: Try a lower pool
	jns	.1			;    As long as there are lower pools
	int	0xFF			;Crash
.2:	mov	[count+4*ebx], ecx	;Update count
	shl	ebx, 12			;Convert pool number to pool address
	add	ebx, pool
	and	ecx, 1023		;Index within pool page
	mov	ecx, [ebx+ecx*4]	;Physical page from that index
	jnz	.3			;Jump if not set boundary
	sar	ebx, 10			;Convert to address of PTE that
					;  maps this pool
	xchg	ecx, [ebx]		;Map next set's page zero
					;eax = page zero of consumed set
.3:	mov	cl, 0x63		;Make it into a page table entry
	mov	[edx], ecx		;Map it
	mov	ebx, cr3		;Flush TLB
	mov	cr3, ebx
	pop	ebx
	ret
