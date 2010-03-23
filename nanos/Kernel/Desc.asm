desc:

;===============================================================================
;
;	desc.create_data		(eax = Selector) == (edx [Settings][Selector], ecx Size)

	;Create a Data/Code Segment
	
	;Selector
		;bit0:1 = DPL
		;bit2 = ti, 0:GDT 1:LDT
		;bit3-15 = selector, 0 = any selector
	;Size(in bytes) - Granulary bit is fixed automatically
	;Settings
		;Data        0EWA
		;Code        1CRA

.create_data:

	;Check Privileges
	mov	ax, [caller_cs]
	arpl	dx, ax
	
	;Make Settings
	mov	ax, dx
	and	ax, 3	;keep DPL
	shl	ax, 5
	or	ax, 4090h		;Default 32, Present bit, Data/Code descriptor
	shl	eax, 10h
	or	eax, edx
	and	eax, 40FFFFFFh	;safe settings
	;eax = [Settings | Selector]
	
	push	ebx
	push	ecx	
	push	edx
	
		;ecx = Size
		call desc.create	;(eax Selector, edx = Segment Base, ebx = Desc Base) == (eax [Settings][Selector], ecx Size)
		jc	.create_data_error
	
	pop	edx
	pop	ecx
	pop	ebx
	
	ret
	
		.create_data_error:
	pop	edx
	pop	ecx
	pop	ebx
	stc
	ret

;===============================================================================

;desc.create	;(eax Selector, edx = Segment Base, ebx = Desc Base) == (eax [Settings][Selector], ecx Size)

;Create a data/code/LDT/TSS descriptor

.create:
	;in:
		;eax selector
			;bit0:1 = 00
			;bit2 = ti, 0:GDT 1:LDT
			;bit3-15 = selector, 0 = any selector
		;edx Settings
				;Data        -D-L----1Pl10EWA
				;Code        -D-L----1Pl11CRA
				;LDT         ---A----1Pl00010
				;TSS         ---A----1Pl010B1
		;ecx = Size(in bytes) - Granulary bit is fixed automatically
		;		if ecx exceeds maximum limit without Granulary bit set(4kB steps)
		;		descriptor will automatically use Granulary bit
		;		and then the descriptor limit will be to the next 4kB boundary		
	;out:
		;cf clear,eax = selector
		;cf set  ,eax = error code		
			;ecx = Segment size
			;edx = Segment base
			;ebx = Descriptor base

	
	mov	edx, eax
	and	edx, 50FF0000h
	or	edx, 00800000h		;Present bit
	;edx = settings
	
	call	.create_desc	;(eax = selector, ebx = Desc Base) == (eax = selector)
	jc	.create_error
	or	eax, edx
	
	call	.alloc_mem	;(edx = Base, ecx = Size) == (ecx = Size(even 4kB) )
	jc	.create_error_desc
	call .write_desc	;(same) == (eax = [Settings][Selector], ebx = Desc Base, ecx = Size, edx = Base)
	
	and	eax, 0FFFCh
	clc
	ret

	.create_error_desc:	
	;remove the allocated descriptor
	mov dword [ebx], 0
	mov dword [ebx+4], 0
	
	.create_error:
	stc
	ret


;===============================================================================	
;
;	call	desc.create_desc_ldt	;(eax = selector, ebx = Desc Base) == (ebx = [LDT][Selector])
;
;	Creates a new descriptor in specified LDT
.create_desc_ldt:
	push	ecx
		mov	eax, ebx
		and	eax, 0FFFCh	;eax = selector
		
		push	edx
			mov	edx, ebx
			shr	edx, 10h
			call	desc.get_ldt	;(ebx = Base, ecx = limit) == (edx = selector)
		pop	edx
		jc near .create_desc_error

	jmp	.spec

;===============================================================================	
;
;	desc.get				;(eax [Settings][Selector]) == (edx Selector)
;
;	Change Data/Code descriptor settings
;
		;Data        0EWA
		;Code        1CRA
.get:
	lar	eax, edx
	jnz	.get_error
	
	bt	eax, 12	;Data/Code bit
	jnc	.get_error
	
	and	eax, 00000F00h
	shl	eax, 8	;
	mov	ax, dx
	clc
	ret
	
.get_error:
	stc
	ret
	
;===============================================================================	
;
;	desc.set				;() == (edx [Settings][Selector])
;
;	Change Data/Code descriptor settings
;
.set:
	push	eax
	push	ebx
		;Check descriptor
		lar	ebx, edx		;Present, legal
		jnz	.set_error
		bt	ebx, 12		;Data/Code segment
		jnc	.set_error
		shr	ebx, 13		;Test privileges
		and	ebx, 011b		;ebx = DPL
		mov	eax, [caller_cs]
		and	eax, 011b		;eax = CPL
		cmp	eax, ebx
		ja	.set_error	;Error: not privileged
		
		;Get Descriptor Base
		call	desc.get_desc_base	;(ebx = Base) == (dx = selector)
		
		;Write settings
		mov	eax, edx
		shr	eax, 10h
		and	al, 0Fh
		and byte [ebx + 5], 0F0h
		or  byte [ebx + 5], al
		
		;Done
	pop	ebx
	pop	eax
	clc
	ret
	
.set_error
	pop	ebx
	pop	eax
	stc
	ret


;===============================================================================	
;
;desc.create_desc	;(eax = selector, ebx = Desc Base) == (eax = selector)
;
.create_desc:
	;creates a new selector in specified table
	;in:	eax = selector
		;bit 0:1 = 00
		;bit 2 = ti, 0:GDT 1:LDT
		;bit 3:15 = selector, 0 = any selector
	;return:
		;eax = selector
		;ebx = linear address to descriptor
		
	push	ecx
		and	eax, 0FFFCh
		
		push	edx
			mov	edx, eax
			call	desc.get_table	;(ebx = Table Base, ecx = Table Limit) == (dx = Selector(TI bit) )
		pop	edx
		jc	.create_desc_error

	.spec:	;Called by desc.create_desc_ldt

		;is selector specified?, else any selector
		test ax, 0FFF8h
		jz   .any			;null selector = any selector

		;ax = selector
		;ecx = table limit
		;ebx = table base
			
		;check if selector is free
		push eax				;save selector

			;check table bounds
			or   eax, 111b ; set bits 0-2 (upper bound of descriptor)
			cmp  eax, ecx
			ja   .out_of_bounds
			;selector within bounds

			;set ebx to point at descriptor
			and  eax, 0FFF8h ;only keep selector index
			add  ebx, eax

		pop  eax

		bts dword [ebx	+ 4], 15	;Present bit
		jc   .create_desc_error_full			;Descriptor already present/taken
	
	pop	ecx
	ret

	.out_of_bounds:
		pop  eax ;restore stack
	pop	ecx
	mov  eax, err_table_bounds
	stc
	ret


.any:
	;set ecx to counter of descriptors
	inc	ecx		;if ecx = ...xxx111b
	shr  ecx, 3	;ecx = number of descriptors in the table
	
	and  eax, 04h	;keep ti bit
	sub  eax, ebx	;step 1/2: make selector

	cmp  ecx, 1
	jbe	.create_desc_error_full

	dec	ecx		;don't count null descriptor

	.find_free
		add  ebx, 8

		bts dword [ebx+4], 15 ;Present bit
		jnc	.create_eax_sel

		loop .find_free

		;error: GDT/LDT full

	.create_eax_sel
		pop	ecx
		add  eax, ebx  ;step 2/2: make selector, eax = the new selector
		clc
		ret
	
	.create_desc_error_full:
		mov  eax, err_table_full
		
	.create_desc_error:
		pop	ecx
		stc
		ret
		

;===============================================================================
;
;	desc.delete		;(eax Status) == (edx Selector)
;
;	Delete a data/code descriptor
;
.delete:	
	push	ebx
	push	ecx

		;Check type
		lar	eax, edx
		jnz	.delete_error
		
		bt	eax, 12		;Data/Code
		jnc	.delete_error	;not a data/code segment
		
		;Dealloc Lineary memory(lin + page)
		call	desc.get_seg_base	;(ebx = Segment Base) == (dx = Selector)
		lsl	ecx, edx
		inc	ecx
		call	desc.delete_lin	;(eax Status) == (ebx Base, ecx Size)

		call	desc.delete_desc
		
		.delete_error
	pop	ecx
	pop	ebx
	clc
	ret

;===============================================================================
;
;	desc.delete_module		;() == (edx [Module][Selector])
;
;	Delete a data/code descriptor in specified module
;	TI flag in Selector is ignored
;
.delete_module:
	push	ebx
	push	ecx
	push	edx
		shr	edx, 10h
		call	desc.get_ldt	;(ebx = Base, ecx = limit) == (edx = selector)
	pop	edx
	push	edx
		and	edx, 0FFF8h
		inc	ecx
		and	ecx, 0FFF8h
		cmp	edx, ecx
		ja	.delete_module_error
		
		add	ebx, edx	;[ebx] = descriptor
		
		;Test Descriptor
		mov	ecx, [ebx+4]
		and	ecx, 00109000h
		cmp	ecx, 00009000h			;Avail(interface) not set
		jne	.delete_module_error	;Not a data/code segment
		
		;Read Limit
		mov	ecx, [ebx+4]
		mov	cx, [ebx]
		inc	ecx			;ecx = size(without granular)
		shl	ecx, 12
		bt dword [ebx+4], 17h	;Granulary bit
		jc	.delete_module_gran
		shr	ecx, 12
		.delete_module_gran:
		;ecx = size
		
		;Read Base
		mov	dh, [ebx+7]
		mov	dl, [ebx+4]
		shl	edx, 10h
		mov	dx, [ebx+2]
		;edx = base
		
		;Dealloc memory
		xchg	ebx, edx
		call	desc.delete_lin		;() == (ebx Base, ecx Size)
		jc	.delete_module_error
		
		;Remove Descriptor
		mov	[ebx], dword 0
		mov	[ebx+4], dword 0
		
	pop	edx
	pop	ecx
	pop	ebx
	ret
	
		.delete_module_error:
		stc
	pop	edx	
	pop	ecx
	pop	ebx
	ret
;===============================================================================
;
;	desc.delete_desc		;() == (edx Selector)
;
;	Delete the descriptor only, no checks are made
;
.delete_desc:
	;Delete descriptor
	push	ebx
		call	desc.get_desc_base	;(ebx = Base) == (dx = selector)
		
		;Erase, write zeros
		mov dword [ebx], 0
		mov dword [ebx + 4], 0
	pop	ebx

;===============================================================================
;
;	desc.delete_lin		;() == (ebx Base, ecx Size)
;
;	Delete a linear memory area
;
.delete_lin:
	push	edx
		mov	edx, ebx
		call	page.dealloc_lin	;() == (edx = Base, ecx = Size)
		jc	.delete_lin_error
		
		call	desc.free_mem	;() == (ebx = lineary address, ecx = size)
		jc	.delete_lin_error
	pop	edx
	ret
				
	.delete_lin_error:
	pop	edx
	ret
;===============================================================================
;
;desc.get_paddress:	(eax physical address) == (edx Selector, ebx Base, ecx Size)
	;Return the Physical address and check descriptor size
	
.get_paddress:
	push	ebx
	push	edx
	
		call	desc.get_linear_base	;(edx = Linear Base) == (dx = Selector, ebx = Base, ecx = Size)
		jc	.get_paddress_error
		call	page.getpaddress	;(ebx = physical address) == (edx = linear address)
		jc	.get_paddress_error
		
		mov	eax, ebx
	
	pop	edx
	pop	ebx
	ret
	
	.get_paddress_error:
	pop	edx
	pop	ebx
	ret
;===============================================================================

;desc.get_linear_base	;(eax = Linear Base) == (edx = Selector, ebx = Base, ecx = Size)
 	    
.get_linear_base:
	;Get linear base from Selector
	;where to allocate/deallocate memory
	;also check limits

	;in:
	;    dx = Selector
	;   ebx = Base
	;   ecx = Size
	;out:
	;	edx = linear base
	;	cf set if segment is too small

	push	edx
	push	ecx
	push	ebx
			
		call	desc.get_table	;(ebx = Table Base, ecx = Table Limit, eax = changed) == (dx = Selector(TI bit) )
		jc	.limit_not_ok			;Table error: LDT does not exist
		
		mov	eax, edx
		and	eax, 0000FFF8h			;Mask off TI and RPL bits
		add	ebx, eax				;ebx = Linear address to Descriptor
			
		;Check present bit
		mov	eax, [ebx + 4]
		bt	eax, 15
		jnc	.get_linear_base_error
		
		;Check if it is data or code
		bt	eax, 12
		jc	.linear_is_data
		
		;Check if it is LDT or TSS
		bt	eax, 10
		jnc	.linear_is_data
		
	;Error: Descriptor is not of a type allowed
		.get_linear_base_error:			
	pop	ebx
	pop	ecx
	pop	edx
	mov	eax, err_page_descriptor
	stc
	ret
			
		.linear_is_data:

		;get descriptor limit
		mov	ax, [ebx + 6]
		shl	eax, 10h
		mov	ax, [ebx]
		
		;test granulary bit
		rol  eax, 12			;removes unwanted bits
		bt	eax,	4
		jc	.gran_bit
		
		shr  eax, 12			
		jmp short .got_desc_limit
		
		.gran_bit:
		or	eax, 0FFFh
		
		.got_desc_limit:

		mov	edx, ebx		;save ebx for the future

		;edx = Linear address to segment
		;eax = descriptor limit
		;ecx = allocation size
			
	pop	ebx			;ebx = base inside descriptor
	pop	ecx			;ecx = allocation size
	push	ecx
	push	ebx
	
		;check descriptor size
		cmp	ebx, eax
		ja	.limit_not_ok
		add  ebx, ecx
		inc	eax			;eax = size
 		cmp	ebx, eax
		jbe	.limit_ok
		
		.limit_not_ok:
	;error, page count is larger than descriptor limit
	pop	ebx
	pop	ecx
	pop	edx
	mov	eax, err_page_limit
	stc
	ret
		
	.limit_ok:
	
					;edx = Linear address to Descriptor
	
	;get linear base from descriptor
	mov	eax, [edx + 2]
	shl	eax, 8
	mov	al, [edx + 7]
	ror	eax, 8
					;eax = linear base from descriptor
	
	pop	ebx			;ebx = base in descriptor
	pop	ecx
	pop	edx
	
	add	eax, ebx		;eax = linear base for page to work at
	clc
	ret

;===============================================================================

;desc.get_table	;(ebx = Table Base, ecx = Table Limit) == (dx = Selector(TI bit) )

.get_table:
	;get Table base + limits
	;in:	dx = selector(uses only TI bit)
	;out:
	
	;get which table
	bt	dx, 2	 ;check TI bit
	jnc   .get_gdt
	
	;Get LDT
		push	edx
			sldt	dx
			call	desc.get_ldt	;(ebx = Base, ecx = limit) == (edx = selector)
		pop	edx
		clc
		ret
		
	.get_gdt:
		;get GDT base+length
		sub	esp, byte 8		;Make room for Base and limit to be written into d_table

			sgdt [d_table]
	
			movzx ecx, word [d_table_lim]		;ecx = table limit
			mov	 ebx, [d_table_base]		;ebx = table base
	
		add	esp, byte 8			;return stack space

		clc
		ret
		
		
;===============================================================================
;
;	desc.get_ldt	;(ebx = Base, ecx = limit) == (edx = selector)
.get_ldt:
	and	edx, 0000FFFFh
	;get LDT selector
	sub	esp, byte 4			;make room for selector
		
		lsl	ecx, edx		;Make sure it is a legal Segment
		jnz	.ldt_error
		
		call desc.get_ldt_base	;(ebx = Base) == (dx = Selector)
		jc	.ldt_error			;Error: Selector does not point at a LDT descriptor
		
	add	esp, byte 4		;return stack space
	ret
	
		.ldt_error:
	add	esp, byte 4		;return stack space
	
	;mov	eax, err_desc_LDT does not exist
	stc
	ret

		
;===============================================================================
;
;desc.get_desc_base	;(ebx = Base) == (dx = selector)

.get_desc_base:
	push	edx
	push	ecx
		;Get Table Base
		call	desc.get_table	;(ebx = Table Base, ecx = Table Limit) == (dx = Selector(TI bit) )
		
		and	edx, 0000FFF8h
		or	edx, 0111b
		cmp	edx, ecx
		ja	.get_desc_base_error
		and	edx, 0FFF8h
		
		add	ebx, edx
	pop	ecx
	pop	edx	
	clc		
	ret
	
	.get_desc_base_error:
	pop	ecx
	pop	edx	
	stc
	ret
				
;===============================================================================
;
;desc.write_desc	;(same) == (eax = [Settings][Selector], ebx = Desc Base, ecx = Size, edx = Base)


.write_desc:	
	;Write data to selector
	;ebx = linear pointer to descriptor in the table
	;eax = 16:31	Settings
		;Data        -D-L----1Pl10EWA
		;Code        -D-L----1Pl11CRA
		;LDT         ---A----1Pl00010
		;TSS         ---A----1Pl010B1
	;	0:15		Selector(including TI bit) - to be returned
	;ecx = size
	;edx = base
	push	ecx
	pushfd
	cli
	
	;write all data into descriptor
	mov	[ebx + 4], edx
	mov	[ebx + 2], edx		;Base done
	
	sub	ecx, 1			;ecx = limit = size - 1
	jc	.write_desc_error	;if size is zero
		
	;check limit size, use granulary bit?
	test	ecx, 0FFF00000h
	jz	.write_limit
	
	shr	ecx, 12			;return size in 4kB chunks
	bts	eax, 31			;set Granulary bit
	
	.write_limit:
	mov	[ebx]    , cx		;Limit 0:15
	shr	ecx, 10h
	mov	[ebx + 6], cl		;Limit16:19
	
	mov	ecx, eax
	shr	ecx, 10h
	and	cx, 0F0FFh
	and word [ebx + 5], 0F00h
	or	[ebx + 5], cx		;Settings done
	
	;ax=selector
	popfd
	pop	ecx
	clc
	ret

.write_desc_error:
	popfd
	pop	ecx
	stc
	ret

;===============================================================================

;desc.get_ldt_base	;(ebx = Base) == (dx = Selector)

.get_ldt_base:
	;Same as desc_get_base, except settings is destroyed
	;Make sure the descriptor is a LDT
		;LDT         ---A----1Pl00010
	call	desc.get_seg_base_type	;(ebx = Segment Base, edx = [Settings][Selector]) == (dx = Selector)
	jc	.get_ldt_base_error

	;check type
	ror	edx, 10h
	and	dx, 10011111b		;bits to check
	cmp	dx, 10000010b		;equal?
	jne	.get_ldt_base_error
	
	shr	edx, 10h
	clc					;it is a LDT
	ret
	
	.get_ldt_base_error:
	shr	edx, 10h
	stc
	ret
	
	
;===============================================================================

;desc.get_tss_base	;(ebx = Base) == (dx = Selector)

.get_tss_base:
	;Same as desc_get_base, except settings is destroyed
	;Make sure the descriptor is a TSS
		;Data        -D-L----1Pl10EWA
		;Code        -D-L----1Pl11CRA
		;LDT         ---A----1Pl00010
		;TSS         ---A----1Pl0S0B1
	call	desc.get_seg_base_type	;(ebx = Segment Base, edx = [Settings][Selector]) == (dx = Selector)
	jc	.get_ldt_base_error

	;check type
	ror	edx, 10h
	xor	dx, 00010100b		;make zeros to ones, ones to zeros
	and	dx, 10011101b		;bits to check
	cmp	dx, 10011101b		;equal?
	jne	.get_ldt_base_error
	
	shr	edx, 10h
	clc					;it is a TSS
	ret

;===============================================================================
;
;desc.get_seg_base	;(ebx = Segment Base) == (dx = Selector)
;
.get_seg_base:

	push	eax
	push	ecx
		xor	ebx, ebx
		xor	ecx, ecx
		call	desc.get_linear_base	;(eax = Linear Base) == (edx = Selector, ebx = Base, ecx = Size)
		mov	ebx, eax
	pop	ecx
	pop	eax
	ret
	
;===============================================================================
;
;desc.get_seg_base_type	;(ebx = Segment Base, edx = [Settings][Selector]) == (dx = Selector)
;get descriptor base and type

.get_seg_base_type:
	
	push	ecx
	push	edx
	
		;get table base
		bt	dx, 2	 ;check TI bit
		jnc   .get_base_gdt
		;jmp  .get_base_ldt

	;get LDT Base
		.get_base_ldt:
		sldt dx			;stores a selector to the LDT
		call desc.get_ldt_base	;(ebx = Base) == (dx = Selector)
		jc	.get_base_error_pop

		lsl	ecx, edx
	
		jmp	.get_base_spec

		.get_base_error_pop:
		
	.get_base_error:
	pop	edx
	pop	ecx
	stc
	ret
			
		.get_base_gdt:
		;get GDT base
		sub	esp, byte 8		;Make room for Base and limit to be written into d_table
			sgdt [d_table]
			mov	 ebx, [d_table_base]		;ebx = table base
		add	esp, byte 8			;return stack space

		.get_base_spec:	

		;Check Descriptor
		lar	ecx, edx
		jnz	.get_base_error	;Descriptor problem: not present, illegal type, out of table limits...
		
		bt	ecx, 15
		jnc	.get_base_error	;Descriptor not present
		
		;Check if it is data or code
		bt	ecx, 12			;Bit is 1 for Data and Code
		jc	.is_data
		
		;Check if it is LDT or TSS
		bt	ecx, 10			;Bit is 0 for LDT and TSS
		jc	.get_base_error
		
		.is_data:

		;set ebx to point at descriptor
		and  edx, 0FFF8h 		;only keep selector index
		add  ebx, edx			;ebx = Pointer at Descriptor
		
		;get descriptor base into ebx
		mov	ecx, ebx
		mov	ebx, [ecx + 2]
		rol	ebx, 8
		mov	bl, [ecx + 7]
		ror	ebx, 8
		;ebx = descriptor base
	
	pop	edx
		
	ror	edx, 10h
	mov	dx, [ecx + 5]
	rol	edx, 10h			;edx = [Settings][Selector]
		
	pop	ecx

	clc
	ret
		
		
;==============================================
;==============================================
;=========					============
;=========	Memory Management	============
;=========					============
;==============================================
;==============================================
		
		
		
		
		
		
		
		
;===============================================================================

;desc.alloc_mem	;(edx = Base) == (ecx = Size)

.alloc_mem:
	;get linear memory from Free Linear Memory list
	;in:  ecx = size in bytes
	;out: edx = linear pointer
	;memory is reserved in 4kB chunks
	push	eax
	push	ebx
	push	ecx
	
	add	ecx,      0FFFh
	and	ecx, 0FFFFF000h	;4kB boundary
	
	mov	ebx, lin_FLM * pages
	
	.check_size:
		;Check size of FLM Entry
		mov	eax, [ebx + 4]
		sub	eax, [ebx]		;eax = size of free memory
		jz	.err_out_of_memory
		cmp	eax, ecx
		ja	.get_lin			;entry is larger than wanted memory
		je	.get_lin_entry		;entry size = requested size
		
		;go to the next entry
		add	ebx, 8
		
		jmp .check_size

		.err_out_of_memory:
			;Error: not enough memory:
			;	1: out of lineary memory
			;	2: no entry with the specified size(lineary memory is too fragmented)
			pop	ecx
			pop	ebx
			pop	eax
			stc
			ret
			
	.get_lin:
	;current FLM entry is big enough
	
	mov	edx, [ebx]	;edx = linear base for memory taken from the FLM table
	add	[ebx], ecx
	jmp near .done
	
	.get_lin_entry:
	;requested size = current entry size

	mov	edx, [ebx]	;edx = linear base for memory taken from the FLM table

	;current entry is empty, move all following entries one step back over current entry
	
	.move_FLM:
		;move the next entry one step back
		mov 	eax, [ebx + 8]
		mov	[ebx], eax
		mov	eax, [ebx + 12]
		mov	[ebx + 4], eax
		
		add	ebx, byte 8
		
		cmp dword [ebx], byte 0
		jne	.move_FLM
	
	.done:
	
	pop	ecx
	pop	ebx
	pop	eax
	clc
	ret

	
;===============================================================================

;desc.free_mem	;() == (ebx = lineary address, ecx = size)

.free_mem:
	;return linear memory to FLM
	;in:  ebx = linear pointer
	;     ecx = size in bytes
	;memory is returned in 4kB chunks
	
	push	ebx
	push	ecx
	push	edx
	
	mov	edx, ebx
	
	add	ecx, edx
	add	ecx,      0FFFh
	and	ecx, 0FFFFF000h	;4kB boundary
	and	edx, 0FFFFF000h	;4kB boundary
	
	;linear memory to return
	;edx = start
	;ecx = end
	
	mov	ebx, lin_FLM * pages
	
	.find_area:
		cmp	ecx, [ebx]
		je	.add_to_entry_start
		jb	.insert_before_entry
		cmp	edx, [ebx + 4]
		jbe	.add_to_entry_end
		
		;check next
		add	ebx, 8
		cmp	ebx, (lin_FLM + size_FLM) * pages - 8
		jae	$					;FLM table is too small
		jmp	.find_area		
		
		
	.add_to_entry_start:
		mov	[ebx], edx
		jmp	.return_free_lin_done
	
	.insert_before_entry:

		;move all entries one step forward
		xchg	edx, [ebx]
		xchg ecx, [ebx + 4]
		
		cmp	ecx, 0
		je	.return_free_lin_done
		
		add	ebx, 8
		cmp	ebx, (lin_FLM + size_FLM) * pages - 8
		jae	$					;FLM table is too small
		
		jmp	.insert_before_entry
			
	.add_to_entry_end:
		cmp	[ebx + 8], ecx
		jne	.add_to_end
		mov	ecx, [ebx + 8]
		
		.move_back:
		;move remaining entries one step back
			mov	edx, [ebx +  8]
			mov	ecx, [ebx + 0Ch]
			mov	[ebx], edx
			mov	[ebx], ecx
			
			cmp	ecx, 0
			je	.return_free_lin_done
			
			add	ebx, 8
			cmp	ebx, (lin_FLM + size_FLM) * pages - 8
			jae	$					;FLM table is too small - this should never occur here
			
			jmp	.move_back
		
		.add_to_end
		mov	[ebx + 4], ecx
;		jmp	.return_done
			
	.return_free_lin_done:
	pop	edx
	pop	ecx
	pop	ebx
	clc
	ret
	