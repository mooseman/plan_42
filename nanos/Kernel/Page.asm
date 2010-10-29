
;Page management
page:

;===============================================================================
;
;page.freemem		;(eax Free memory) == ()
;	Get free physical memory
;
.freemem:
	push	ebx
		mov	ebx, [System.lin_FPT]
		mov	eax, [ebx]		;get address to last page
		shl	eax, 12 - 2		;12: size of page, 4: size of page entry
	pop	ebx
	clc
	ret
	

;===============================================================================
;
;page.alloc	;() == (edx = [Settings][Selector], ebx = Base, ecx = Size)

.alloc:
	;allocates one or more pages to a memory area specified by a descriptor
	;in:
	;   eax = 0:15 Selector
	;		16	-
	;		17	Writeable
	;		18	User
	;		19	PWT - Page Write-through
	;		20	PCD - Page Cache Disabled
	;   ebx = Base
	;   ecx = size

	push	eax
	push	ecx
	push	edx

		cmp	ecx, 0
		je	.alloc_done

		call	desc.get_linear_base	;(eax = Linear Base) == (edx = Selector, ebx = Base, ecx = Size)
		jc	.alloc_error
			
	 	add	ecx, 0FFFh
		shr	ecx, 12			;ecx = number of pages
		
		xchg	eax, edx
		
		shr	eax, 10h
		and	eax, 011110b
		or	eax, 000001b		;Present bit

		;.burst:
		call	page.popfree_burst		;() == ;(eax = Settings, edx = Linear Address, ecx = pages)
		jc	.alloc_error
	
		.alloc_done:
		clc
	.alloc_error:
	pop	edx
	pop	ecx
	pop	eax
	ret

;===============================================================================

;page.popfree_burst		;() == ;(eax = Settings, edx = Linear Address, ecx = pages)

	;Allocate ecx pages starting at ebx

.popfree_burst:	;allocate free pages
	push	ecx
	push	es
	push	edi
	push	esi
	pushf
	push	edx
	push	eax
	cli

		;Make sure all PT are allocated
		shl	ecx, 12
		call	.allocate_PT	;() == (edx = linear address, ecx = size)
		shr	ecx, 12
		
		;Check free memory
		mov	esi, [System.lin_FPT]
		mov	eax, [esi]
		shr	eax, 2				;eax = number of pages free
		cmp	eax, ecx
		jb	.no_burst_FreePages
		
		;Move the free pages to the PT
		sub	eax, ecx
		shl	eax, 2
		mov	[esi], eax				;FPT Stack pointer
		
		add	esi, eax
		add	esi, 4			;first free page to get
		mov	edi, edx
		and	edi, 0FFFFF000h
		shr	edi, 10			;edi = address at first PTE
		mov	ax, ds
		mov	es, ax
	pop	edx
	push	edx
		.burst_allocate:
			lodsd
			or	eax, edx
			stosd
			
			;invalidate TLB
			mov	eax, edi
			shr	eax, 10
			invlpg	[eax]
			
		loop	.burst_allocate
				
		
	;Done
	pop	eax
	pop	edx
	popf
	pop	esi
	pop	edi
	pop	es
	pop	ecx
	clc
	ret
	
	.no_burst_FreePages_pop:
	pop	edx
	pop	ecx
	.no_burst_FreePages:
	pop	eax
	pop	edx
	popf
	pop	esi
	pop	edi
	pop	es
	pop	ecx
	;no enough(ecx) free pages left - out of memory
	stc
	ret

	

;===============================================================================

;page.popfree	;(eax = Free PTE) == ()
    
.popfree:	;get one free page
	pushf
	push	ebx
		cli
		
		mov	ebx, [System.lin_FPT]
		cmp	dword [ebx], 0
		je	.noFreePages
		
		sub	dword [ebx], 4				;FPT Stack pointer
		add	ebx, [ebx]
		mov	eax, [ebx + 4]
	
	pop	ebx
	popf
	or	eax, 1					;set present bit
	clc
	ret
	
		.noFreePages:
	pop	ebx
	popf
	;no fre pages left - out of memory
	mov	eax, 0
	stc
	ret

;===============================================================================
;
;page.dealloc	;() == (edx = Selector, ebx = Base, ecx = Size)

;;;;
;
;	    INVLPG is necessary in all cases if the old PTE/PDE state was "present", and not necessary otherwise.
;
;;;;
;    An optimization for freeing the PTEs:
;
;    - put the free PTEs not to free list, but to "dirty" list.
;    - PTEs cannot be reallocated from the "dirty" list
;    - when "dirty" list becomes too long - execute
;
;        mov    eax, cr3
;        mov    cr3, eax
;
;    and then move the "dirty" list to the free list".
;
;    This saves time on INVLPG.
;;;;
	
.dealloc:
    ;Deallocates...

	push	eax
	push	ecx
	push	edx
	        
		call	desc.get_linear_base	;(eax = Linear Base) == (edx = Selector, ebx = Base, ecx = Size)
		jc	.dealloc_done

		mov	edx, eax
		call	page.dealloc_lin	;() == (edx = Base, ecx = Size)
				
		clc
	.dealloc_done:
	pop	edx	
	pop	ecx
	pop	eax
    	ret

;===============================================================================
;
;page.dealloc_lin	;() == (edx = Base, ecx = Size)
.dealloc_lin:
	push	ecx
	push	edx
	        
		cmp	ecx, 0
		je	.dealloc_lin_done
		
		add	ecx, edx
		and	edx, 0FFFFF000h
		sub	ecx, edx
		dec	ecx
		shr	ecx, 12			;ecx = number of pages
		inc	ecx
		
		.daealloc_loop:
		
			call .removepage	;() == (edx = Linear Address)
			add	edx, 1000h
		
		loop .daealloc_loop
		
		clc
	.dealloc_lin_done:
	pop	edx	
	pop	ecx
    	ret

;===============================================================================

;page.removepage	;() == (edx = Linear Address)

.removepage
	;edx = linear base
	push	eax
		
		call .readpage		;(eax = PTE) == (edx = Linear Address)
		jc	.removepage_done
		
		call .pushfree		;(eax = 0) == (eax = PTE to free)
		
		call .writepage	;(edx = Linear Address, eax = PTE to write) == (edx = Linear Address, eax = PTE to write)
		
	.removepage_done:
	pop	eax
	clc
	ret

;===============================================================================

;page.pushfree	;(eax = 0) == (eax = PTE to free)
	
.pushfree: ;push one page to the free page stack
	;in:		eax = page to return
	;out:	eax = 0
	push	ebx
		mov	ebx, [System.lin_FPT]
		add	dword [ebx], 4				;FPT Stack pointer

		and	eax, 0FFFFF000h			;Remove settings
		
		add	ebx, [ebx]
		mov	[ebx], eax				;return Page to stack
		
		pop	ebx
	mov	eax, 0
	clc
	ret
	
	
;===============================================================================

;page.readpage	;(eax = PTE) == (edx = Linear Address)

.readpage:
	;get page at linear address
	;edx = linear base
	;return eax = page at address

	push	ebx
	
	call	.get_PTE_address	;(ebx = PTE Pointer, edx = Linear Address) == (edx = Linear Address)
	jc	.readpage_error
	mov	eax, [ebx]	
	
	pop	ebx
	clc
	.readpage_error:
	ret

;===============================================================================

;page.writepage	;(edx = Linear Address, eax = PTE to write) == (edx = Linear Address, eax = PTE to write) ! ("Out of memory")
	
.writepage:
	;edx = linear base
	;eax = page to write
	
	push	ebx

		call	.get_PTE_address	;(ebx = PTE Pointer, edx = Linear Address) == (edx = Linear Address)
		jc	.writepage_error
		
		;if page already exists?
		cmp	[ebx], dword 0
		je	.write_page_now
			
			;free page
			push	eax
			mov	eax, [ebx]
			call	.pushfree	;(eax = 0) == (eax = PTE to free)	
			pop	eax
		
		.write_page_now:
		
		;write page(eax)
		mov	[ebx], eax

		;invalidate TLB
		shr	ebx, 10
		invlpg	[ebx]

		
	pop	ebx
	clc
	ret
	
	.writepage_error:
	pop	ebx
	stc
	ret

	
;===============================================================================

;page.get_PTE_address	;(ebx = PTE Pointer) == (edx = Linear Address)
	;get address to the PTE of the linear address
	;if PT doesn't exist it is created
	
	;in:		edx = linear address
	;error:	out of memory

.get_PTE_address:
	call	.create_PT	;() == (edx = Linear Address)
	jc	.get_PTE_address_error
	
	mov	ebx, edx
	and	ebx, 0FFFFF000h
	shr	ebx, 10
	clc
	ret
	
.get_PTE_address_error:
	stc
	ret

	
;===============================================================================

;page.allocate_PT	;() == (edx = linear address, ecx = size)
	;Create a PT(if needed) for PT at linear address
	
	;in:		edx = linear address
	;error:	out of memory
.allocate_PT:
	
		;Make sure all PT are allocated
	push	eax
	push	ecx
	push	edx
		and	edx, 0FFFFF000h
		mov	eax, edx		
		and	eax, 0FFC00000h	;eax = start PT address
		add	edx, ecx
		sub	edx, eax
		mov	ecx, edx
		shr	ecx, 22	
		inc	ecx				;ecx = number of PT to check
		mov	edx, eax			;edx = linear address at first memory of the first PT
		
		;Check size
		mov	eax, ecx
		shl	eax, 22	;eax = PT
		add	eax, edx
		cmp	eax, edx
		jb	.allocate_PT_error
		
		
		.check_PT:
		call	page.create_PT	;() == (edx = Linear Address)
			jc	.no_burst_FreePages_pop
			add	edx, 400000h
		loop	.check_PT
	pop	edx
	pop	ecx
	pop	eax
	
	clc
	ret
	
	.allocate_PT_error:
	pop	edx
	pop	ecx
	pop	eax

	stc
	ret
	
;===============================================================================

;page.create_PT	;() == (edx = Linear Address)
	;Create a PT(if needed) for PT at linear address
	
	;in:		edx = linear address
	;error:	out of memory

.create_PT:
	push	ebx	

		;Check if PT exist
		mov	ebx, edx
		shr	ebx, 20			;Get Page Table index
		and	ebx, 0FFCh		;ebx = mem pointer at PD Entry
		bt dword [ebx], 0		;Check if PT Entry  exists
		jc	.create_PT_done

		;ignore size - assume 4kB
					
		;Page Table doesn't exist, make one
		;[ebx] = page table entry
		push	eax
		
			;get a free page
			call	.popfree	;(eax = Free PTE) == ()
			jc	.create_PT_error
			
			;write to table
			mov	[ebx], eax
			
			;invalidate TLB
			shr	ebx, 10
			invlpg	[ebx]

			;write zeros to table
			push edi
			push ecx
			push	es
			
				mov	edi, edx
				and	edi, 0FFC00000h
				shr	edi, 10
				mov	ecx, 400h		;dwords in a PT
				mov	ax, ds
				mov	es, ax
				mov	eax, 0
				rep	stosd		;write zeros to table
			
			pop	es
			pop	ecx
			pop	edi
		
		pop	eax
		
	.create_PT_done:
	pop	ebx
	clc
	ret
		
	  .create_PT_error:
	  	pop	eax
	  	pop	ebx
		;out of memory cant get address
		;Can't create PD entry for specified address
		stc
		ret

		
;===============================================================================

;page.getpaddress	;(ebx = physical address) == (edx = linear address)
;	Get physical address from linear address
.getpaddress
	push	edx
		call	.get_PTE_address	;(ebx = PTE Pointer) == (edx = Linear Address)
		mov	ebx, [ebx]
		and	ebx, 0FFFFF000h
		and	edx, 000000FFFh
		or	ebx, edx
	pop	edx

;===============================================================================
;
;page.alias	;() == (int Source Selector, int Source Base, int Size, int Target Selector, int Target Base)
;
;	Map a linear memory area to the same physical memory as another lineary memory
;
; .alias:
; 	push	ebx
; 	push	ecx
; 	push	edx
; 	push	esi
; 	push	edi
; 		
; 		;Source
; 		mov	ax, [param1]
; 		mov	ebx, [param2]

; 		mov	ecx, [param3]	;ecx = size
; 		add	ecx, 0FFFh
; 		shr	ecx, 12
; 		
; 		call	desc.get_linear_base	;(edx = Linear Base, ax = Selector, ebx = Base, ecx = Size) == (ax = Selector, ebx = Base, ecx = Size) ! ("Out of Bounds")
; 		jc	.alias_error
; 		mov	esi, edx	;esi = source linear address
; 		shr	esi, 10
; 		and	esi, 3FFFFCh
; 			
; 		;Target
; 		mov	ax, [param4]
; 		mov	ebx, [param5]
; 		call	desc.get_linear_base	;(edx = Linear Base, ax = Selector, ebx = Base, ecx = Size) == (ax = Selector, ebx = Base, ecx = Size) ! ("Out of Bounds")
; 		jc	.alias_error
; 		mov	edi, edx	;eid = target linear address
; 		shr	edi, 10
; 		and	edi, 3FFFFCh

; 		push	es			
; 			mov	ax, ds
; 			mov	es, ax
; 				
; 			rep	movsd
; 		pop	es
; 	
; 	pop	edi
; 	pop	esi
; 	pop	edx
; 	pop	ecx
; 	pop	ebx
; 	
; 	xor	eax, eax
; 	clc
; 	ret
; 	
; 	
; 	.alias_error:
; 	pop	edi
; 	pop	esi
; 	pop	edx
; 	pop	ecx
; 	pop	ebx
; 	
; 	mov	eax, -1
; 	stc
; 	ret
	

;===============================================================================
;
;page.phys_allocate		;() == (edx Selector, ebx Base, edi First PTE, ecx Size)
;
;	Allocate lineary memory with specified physical memory
;
.phys_allocate:
	push	eax
	push	edi
		
		call	desc.get_linear_base	;(eax = Linear Base) == (edx = Selector, ebx = Base, ecx = Size)
		jc	.phys_allocate_error

		and	edi, 0FFFFF01Ah
		or	edi, 1
		
		call	.pallocate	;() == (eax Linear Base, edi First PTE, ecx Size)
		jc	.phys_allocate_error

	pop	edi
	pop	eax		
	clc
	ret
		
	.phys_allocate_error:
	pop	edi
	add	esp, 4	;pop	eax		;Keep eax=error code
	stc
	ret

;===============================================================================

;page.pallocate	;() == (eax Linear Base, edi First PTE, ecx Size)
;	Allocate lineary memory with specified physical memory

.pallocate:
	push	edx
	push edi
	push ecx
	
		mov	edx, eax
		call	.allocate_PT	;() == (edx = linear address, ecx = size)
		jc	.pallocate_error
	
		or	edi, 1		;Set present bit
		shr	edx, 10
		and	edx, 3FFFFCh	;edx = PTE address
		add	ecx, 0FFFh
		shr	ecx, 12		;ecx = pages
		
		.pallocate_loop:
		mov	[edx], edi
		add	edx, 4
		add	edi, 1000h
		loop	.pallocate_loop

	pop	ecx
	pop	edi
	pop	edx
		
	clc
	ret

	.pallocate_error:
	pop	ecx
	pop	edi
	pop	edx
		
	stc
	ret
