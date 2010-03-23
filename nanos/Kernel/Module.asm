;
; Module management
;
;
module:

;===============================================================================
;
;	module.create		;(eax Selector) == (ecx LDT Size)
;	Create A module

.create:
	push	ebx
	push	ecx
	push	edx
		;Size
		shl	ecx, 3	;8 bytes per desc
				
		;Descriptor
		xor	eax, eax	
		call	desc.create_desc	;(eax = selector, ebx = Desc Base) == (eax = selector)
		jc near .create_error
		
		push	eax	;save selector

			;Linear memory
			add	ecx, module_struc.LDT
			call desc.alloc_mem	;(edx = Base, ecx = Size) == (ecx = Size(even 4kB) )
			jc	.create_error_desc

			;Pages
			mov	eax, 3	;Present, writeable
			push	ecx
				dec	ecx
				shr	ecx, 12
				inc	ecx
				call	page.popfree_burst		;() == ;(eax = Settings, edx = Linear Address, ecx = pages)
			pop	ecx
				jc	.create_error_lin
			
			;Write Module data
			push	edi
			push	ecx
				;Name
				mov dword [edx+module_struc.Name+0], 'new '
				mov dword [edx+module_struc.Name+4], 'unna'
				mov dword [edx+module_struc.Name+8], 'med '
				mov dword [edx+module_struc.Name+12], 'mod '
				
				;Interface list
				mov	eax, 0
				mov	ecx, module_struc.InterfaceSize / 4
				mov	edi, edx
				add	edi, module_struc.Interface
				push	es
					push	ds
					pop	es
					rep	stosd
				pop	es
			pop	ecx
			pop	edi
							
		pop	eax	;retrieve descriptor
				
		;Write Desc
		sub	ecx, module_struc.LDT
		add	edx, module_struc.LDT
		or	eax, 00820000h		;LDT		---A----1Pl00010 00000000 00000000
		call desc.write_desc	;(same) == (eax = [Settings][Selector], ebx = Desc Base, ecx = Size, edx = Base)
		
	pop	edx	
	pop	ecx
	pop	ebx
	
	;Done
	and	eax, 0FFFCh
	clc
	ret

	
			.create_error_lin:	
			;dealloc lineary memory
			mov	ebx, edx
			call	desc.free_mem	;() == (ebx = lineary address, ecx = size)

			.create_error_desc:	
		pop	eax
		;remove the allocated descriptor
		mov dword [ebx], 0
		mov dword [ebx+4], 0
		
	pop	edx	
	pop	ecx
	pop	ebx
		
	.create_error:
	xor	eax, eax
	stc
	ret

	
	

;===============================================================================
;
;	module.delete		;() == (edx Selector)
;	Delete a module

.delete:
	push	edi
	push ebx
	push	ecx
		and	edx, 0FFF8h	
		mov	edi, edx		;edi = module selector
		
		lar	edx, edi
		and	edx, 00009F00h
		cmp	edx, 00008200h
		jne	.delete_error
		
		;Scan GDT for TSS		
		xor	edx, edx
		call	desc.get_table	;(ebx = Table Base, ecx = Table Limit) == (dx = Selector(TI bit) )
	
		sub	ecx, 7
		shr	ecx, 3
		jz	.delete_scan_GDT_done
		mov	edx, 0
		.delete_scan_GDT:
			add	edx, byte 8
			;check if in correct module(edi)
			call	desc.get_seg_base	;(ebx = Segment Base) == (dx = Selector)
			cmp	[ebx + 60h], edi
			jne	.delete_scan_GDT_not
				;delete Task
				call	mult.delete	;() == (edx Selector)
			.delete_scan_GDT_not:
			loop	.delete_scan_GDT
		.delete_scan_GDT_done:
		;all tasks in module deleted

				
		;Scan LDT for Data/Code
		mov	dl, 4
		call	desc.get_table	;(ebx = Table Base, ecx = Table Limit) == (dx = Selector(TI bit) )
		inc	ecx
		push	ecx
			shr	ecx, 3
			jz	.delete_scan_LDT_done
			
			mov	edx, edi
			shl	edx, 10h
			.delete_scan_LDT:
				;Remove Segment
				call	desc.delete_module		;() == (edx [Module][Selector])

				add	edx, byte 8
				loop	.delete_scan_LDT
			.delete_scan_LDT_done:
		pop	ecx
		;ecx = LDT Size
		;ebx = LDT Base
		
		;Scan Interface list
		push	ebx
		push	ecx
			add	ebx, module_struc.Interface - module_struc.LDT
			mov	ecx, module_struc.InterfaceCount
			.scan_interface:
				mov	edx, [ebx + interface_struc.Type]
				call	interface.delete		;() == (edx Type)
			loop	.scan_interface
		pop	ecx
		pop	ebx
			;Remove all
		
		;Remove Module segment
		call	desc.delete_lin		;() == (ebx Base, ecx Size)
		
		;Remove Module descriptor
		mov	edx, edi
		call	desc.delete_desc		;() == (edx Selector)
	pop	ecx
	pop	ebx
	pop	edi
	clc
	ret

		.delete_error:
		;Not a LDT
	pop	ecx
	pop	ebx
	pop	edi
	stc
	ret
;===============================================================================
;
;module.move		;(eax Selector) == (edx Source selector, ebx Target [module][selector])
;	Move a descriptor from source module to target module

.move:	
	push	edx
	push	ebx
		
	
		;Get source descriptor
		call	desc.get_desc_base	;(ebx = Base) == (dx = selector)
		jc	.move_error
		
		mov	edx, ebx	;Save source Base
		
	pop	ebx
	push	ebx
	
		;Get target descriptor
		call	desc.create_desc_ldt	;(eax = selector, ebx = Desc Base) == (ebx = [LDT][Selector])
		jc	.move_error
		
		;ebx = target
		;edx = source
		push	eax
		pushfd
			cli
			;copy descriptor
			mov	eax, [edx]
			mov	[ebx], eax
			mov	eax, [edx+4]
			mov	[ebx+4], eax		
			;save DPL
			mov	ebx, eax
			
			;clear original descriptor
			xor	eax, eax
			mov	[edx], eax
			mov	[edx+4], eax		
		popfd
		pop	eax
		
		;Adjust RPL in eax to DPL
		shr	ebx, 13
		arpl	ax, bx
		
	pop	ebx
	pop	edx
	
	clc
	ret
	
	.move_error:
	pop	ebx
	pop	edx
	stc
	ret

	
	
;===============================================================================
;
;	module.movetask	;() == (edx Source Task selector, ebx Target Module)	
;	Move a task from source module to target modul

.movetask:
	push	eax
	push	edx
	push	ebx
		
		;Get TSS Base
		push	ebx
			call	desc.get_tss_base	;(ebx = Base) == (dx = Selector)	
			mov	eax, ebx	;eax=TSS Base
		pop	ebx
		jc	.movetask_error
		
		;Get SS0
		mov	edx, [eax + 8]	;ss0 in TSS
			
		;Move SS0
		push	eax
			shl	ebx, 10h
			add	ebx, byte 4	;LDT bit
			call	module.move		;(eax Selector) == (edx Source selector, ebx Target [module][selector])
			mov	edx, eax		;edx = new SS0 selector
		pop	eax		
		jc	.movetask_error
	
		;Save new SS0 selector
		mov	[eax + 8], edx		;Save new SS0 in TSS
		
		;Change LDT in TSS
		shr	ebx, 10h
		mov	[eax + 60h], bx	;Save new LDT in TSS
				
	pop	ebx
	pop	edx
	pop	eax
	clc
	ret
	
	.movetask_error:
	pop	ebx
	pop	edx
	pop	eax
	stc
	ret
