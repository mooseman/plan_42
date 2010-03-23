mult:

;===============================================================================
;
;	mult.create		;(eax = Selector) == ()
;
;	Create a TSS(Process)
;
.create:
	push	edi
	push	ebx		;ss
	push	ecx		;esp
	push	edx		;eip
	
		mov	eax, 00890000h	;eax16:31 = --------1Pl010B1
		mov	ecx, 68h			;size of TSS
		call desc.create	;(eax Selector, edx Segment Base, ebx Desc Base) == (eax [Settings][Selector], ecx Size)
		jc near .create_error
		and	eax, 0FFFCh
		push	eax			;Save Selector

			;fill it with one page
			call	page.popfree	;(eax = Free PTE) == ()
			jc near .tss_error
			or	eax, 0111b	;User,Writeable,Present
			call	page.writepage	;(edx = Linear Address, eax = PTE to wri) == (edx = Linear Address, eax = PTE to write)
			jnc	.create_fill
				
			;Error ;return poped PTE
			call	page.pushfree	;()==(eax = PTE)
			jmp	.tss_error
			
			.create_fill:
		;This page contains:
		
			;fill whole page with zeros
			push	es
				mov	ax, ds
				mov	es, ax
				
				mov	edi, edx
				mov	ecx, 1000h / 4
				xor	eax, eax
				rep	stosd		;Fill TSS with zeros
			pop	es		
			
		;TSS:
					
			;EFlags
			mov dword [edx + 24h], 00001202h	;IOPL=1, IF=1 
						
			;PDBR
			mov	eax, cr3	
			mov	[edx + 1Ch], eax
			
			;LDT, same as callers LDT
			sldt	[edx + 60h]
		
			;IObase
			mov word [edx + 66h], 68h	;at end of TSS
				
		;(Floating point memory, to store float registers during taskswitch) - nothing to write(already zeros)
			;[edx + task_struc.Float], 512 bytes
					
			mov	edi, edx
			
		;Privile 0 Stack
			mov	eax, 0100b	;TI: 1=LDT
			call desc.create_desc	;(eax = selector, ebx = Desc Base) == (eax = selector)
			jc	.tss_error_free
			
			;Data        -D-L----1Pl10EWA
			or	eax, 00920000h
			mov	ecx, pages - task_struc.Stack		;the rest of the page
			add	edx, task_struc.Stack
			call	desc.write_desc	;(same) == (eax = [Settings][Selector], ebx = Desc Base, ecx = Size, edx = Base)
			
			mov	edx, edi
			mov	[edx + 08h], ax		;SS0
			mov dword [edx + 04h], pages - task_struc.Stack - 4	;ESP0

			.tss_no_ss:
		;return selector
		pop	eax
	pop	edx
	pop	ecx
	pop	ebx
	pop	edi
	clc
	ret
	

		.tss_error_free:
			mov	ecx, pages
				
		.tss_error:
		pop	edx	;Selector
		call	desc.get_seg_base	;(ebx = Segment Base) == (dx = Selector)
		call	desc.delete_lin		;() == (ebx Base, ecx Size)
		call	desc.delete_desc		;() == (edx Selector)
		
		.create_error
	pop	edx
	pop	ecx
	pop	ebx
	pop	edi
	stc
	ret

	
;===============================================================================
;
;mult.delete	;() == (edx Selector)
;	Delete a TSS
;
.delete:
	push	ecx
	push	ebx
		;Close interface connected to task
		call	interface.delete_task		;() == (edx Task)
		
		;Check type
		lar	ebx, edx
		jnz	.delete_error
		and	ebx, 00081D00h	;Mask type + Avail bit
		cmp	ebx, 00000900h	;is it a TSS?, Avail = 0 - Not an interface
		jne	.delete_error
		
		;Remove from TL
		call	mult.remove	;(eax Status) == (edx = TSS Selector)

		call	desc.get_seg_base	;(ebx = Segment Base) == (dx = Selector)

		push	edx	
		;Free:
			mov	dx, [ebx + 60h]	;LDT
			shl	edx, 10h
			mov	dx, [ebx + 08h]	;SS0
			call	desc.delete_module		;() == (edx [Module][Selector])
		pop	edx
		
		;Remove segment
		mov	ecx, pages
		call	desc.delete_lin		;() == (ebx Base, ecx Size)
		
		;Remove descriptor
		call	desc.delete_desc		;() == (edx Selector)
		
	pop	ebx
	pop	ecx
	ret

	.delete_error:
	pop	ebx
	pop	ecx
	stc
	ret


;=======================
%macro	move 1
		mov	ebx, [esi + (%1)]
		mov	[es:edi + (%1)], ebx
%endmacro
%macro	moveseg 1
		mov	bx, [esi + (%1)]
		mov	[es:edi + (%1)], bx
%endmacro
%macro	moveseg 2
		mov	bx, [esi + (%1)]
		arpl	bx, %2
		mov	[es:edi + (%1)], bx
%endmacro

;===============================================================================
;
;	mult.get			;() == (edx TSS Selector, ds:ebx TSS segment data)
;
.get:
	push	eax
	push	ebx
	push	esi
	push	edi
	push	es
		mov	eax, [caller_cs]
		
		;Get Target
		mov	edi, ebx
		mov	bx, [caller_ds]
		arpl	bx, ax
		mov	es, bx	;es:edi = Target

		;Get TSS Base
		call	desc.get_tss_base	;(ebx = Base) == (dx = Selector)
		jc near .get_error
		mov	esi, ebx	;ds:esi = TSS segment
		
		;test Module
		sldt	bx
		cmp	bx, [esi + 60h]	;ldt
		jne near .get_error		;TSS LDT must be same as current LDT
		
		;Test CPL
		mov	bl, al	;bl = [caller_cs]
		and	bl, 011b	;bl = CPL
		cmp	bl, 3
		je	.get_3
		cmp	bl, 2
		je	.get_2
		;else CPL = 1, since 0 is kernel only
		
	;CPL 0:
;	[- I/O map    -][-           -]T 64h
;	[-            -][-    LDT     -] 60h
;	[            EFlags            ] 24h
;	[            EIP               ] 20h
;	[-           CR3 PDBR         -] 1Ch
;	[-            -][-   SS0      -] 08h
;	[-            ESP0            -] 04h
;	[-            -][    Link      ] 00h


		.get_1:
		moveseg	10h		;ss2
		move		0Ch		;esp2
				
		.get_2:
		moveseg	18h		;ss2
		move		14h		;esp2
			
		.get_3:
		move	20h	;EIP
		move	28h	;eax
		move	2Ch	;ecx
		move	30h	;edx
		move	34h	;ebx
		move	38h	;esp
		move	3Ch	;ebp
		move	40h	;esi
		move	44h	;edi
		moveseg	48h	;es
		moveseg	4Ch	;cs
		moveseg	50h	;ss
		moveseg	54h	;ds
		moveseg	58h	;fs
		moveseg	5Ch	;gs
	pop	es
	pop	edi
	pop	esi
	pop	ebx
	pop	eax
	clc
	ret
	
	.get_error
	pop	es
	pop	edi
	pop	esi
	pop	ebx
	pop	eax
	stc
	ret

;===============================================================================
;
;	mult.set			;() == (edx TSS Selector, ds:ebx TSS segment data)
;
.set:
	push	eax
	push	ebx
	push	esi
	push	edi
	push	ds
	push	es
		mov	eax, [caller_cs]

		;Get TSS Data
		mov	esi, ebx
		
		;Get Target
		call	desc.get_tss_base	;(ebx = Base) == (dx = Selector)
		jc near .set_error
		push	ds
		pop	es
		mov	edi, ebx	;es:edi = TSS segment

		;Get TSS Data
		mov	bx, [caller_ds]
		arpl	bx, ax
		mov	ds, bx	;ds:esi = Source
		
		;test Module
		sldt	bx
		cmp	bx, [es:edi + 60h]	;ldt
		jne near .set_error		;TSS LDT must be same as current LDT
		
		;Test CPL
		mov	bl, al	;bl = [caller_cs]
		and	bl, 011b	;bl = CPL
		cmp	bl, 3
		je	.set_3
		cmp	bl, 2
		je	.set_2
		;else CPL = 1, since 0 is kernel only
		
	;CPL 0:
;	[- I/O map    -][-           -]T 64h
;	[-            -][-    LDT     -] 60h
;	[            EFlags            ] 24h
;	[            EIP               ] 20h
;	[-           CR3 PDBR         -] 1Ch
;	[-            -][-   SS0      -] 08h
;	[-            ESP0            -] 04h
;	[-            -][    Link      ] 00h

		;moveseg assume eax = [caller_cs]
		.set_1:
		moveseg	10h, ax	;ss2
		move		0Ch		;esp2
				
		.set_2:
		moveseg	18h, ax	;ss2
		move		14h		;esp2
			
		.set_3:
		move	20h	;EIP
		move	28h	;eax
		move	2Ch	;ecx
		move	30h	;edx
		move	34h	;ebx
		move	38h	;esp
		move	3Ch	;ebp
		move	40h	;esi
		move	44h	;edi
		moveseg	48h	;es
		moveseg	4Ch, ax	;cs
		moveseg	50h	;ss
		moveseg	54h	;ds
		moveseg	58h	;fs
		moveseg	5Ch	;gs
	pop	es
	pop	ds
	pop	edi
	pop	esi
	pop	ebx
	pop	eax
	clc
	ret
	
	.set_error
	pop	es
	pop	ds
	pop	edi
	pop	esi
	pop	ebx
	pop	eax
	stc
	ret

;===============================================================================
;
;mult.add	;() == (dx = TSS Selector)

.add:
	push	ebx
	push	ecx
	push	edx
		
		and	edx, 0000FFF8h
		
		;Make sure it is a TSS
		call	desc.get_tss_base	;(ebx = Base) == (dx = Selector)
		jc	.add_error		;Selector is not a TSS Selector
		
		;Make sure it isn't already in the list
		call	.TL_scan	;(ebx = Entry Pointer, ecx = Entries Left) == (dx = TSS Selector)
		jnc	.add_done

		;in:	dx = TSS Selector
		mov	ebx, multitasking_struc.Head
		mov	eax, [ebx+multitasking_struc.last]		;eax = pointer at last entry
		
		;Make sure it isn't the last one = no space left
		add	eax, multitasking_struc.Entry_Size
		cmp	eax, multitasking_struc.Size
		jae	.add_error

		;add to list
		mov	[ebx+multitasking_struc.last], eax	
		mov	ebx, multitasking_struc.Base
		add	ebx, eax
		mov	[ebx+multitasking_struc.current], dx
		mov	[ebx+multitasking_struc.original], dx
		
	.add_done:
	pop	edx
	pop	ecx
	pop	ebx
	;Done, return
	clc
	ret

	.add_error:
	pop	edx
	pop	ecx
	pop	ebx

	mov	eax, err_TL_Full
	stc
	ret


	
;===============================================================================
;
;mult.remove	;(eax Status) == (edx = TSS Selector)

.remove:
	push	ebx
	push	ecx
	push	edx
	push	esi
	push	edi
	pushfd
		cli	;Disable Interrupts
		
		and	edx, 0000FFF8h
		

		call	.TL_scan	;(ebx = Entry Pointer, ecx = Entries Left) == (dx = TSS Selector)
		jc	.remove_error

		;Make sure task is in current ldt(module)
		
		;Test Task type
		cmp	dx, [ebx+multitasking_struc.original]	;interface if not equal
		jne	.remove_error	;can't remove interfaces
						
		;remove process
		
		;move entries
		mov	edx, multitasking_struc.Head
		mov	eax, [edx + multitasking_struc.current]
		add	eax, multitasking_struc.Base
		cmp	eax, ebx
		jbe	.remove_below
		
			;Decrease by one entry
			sub dword [multitasking_struc.Base+multitasking_struc.current], multitasking_struc.Entry_Size
			
		.remove_below:
		;eax = pointer at current
		;ebx = pointer at entry to be removed
		;edx = multitasking_struc.Base
				
		;move the rest one step to the left
		mov	edi, ebx
		mov	esi, ebx
		add	esi, multitasking_struc.Entry_Size
		rep	movsd
		
		;decrease last Entry
		sub dword [edx+multitasking_struc.last], multitasking_struc.Entry_Size
	
	;Test if current
		cmp	eax, ebx	;current equal removed?
		jne	.remove_done
			
			mov	eax, ebx
			sub	eax, multitasking_struc.Entry_Size
			sub	eax, multitasking_struc.Base
			mov	edx, [edx+multitasking_struc.last]
			;eax = current(no base)
			;edx = last(no base)
			call	Multitasking.NextTask
	
	.remove_done:
	popfd	;Enable Interrupts(if it were enabled before)
	pop	edi
	pop	esi
	pop	edx
	pop	ecx
	pop	ebx	
	xor	eax, eax	
	clc
	ret

	.remove_error:
		;TSS not found
		mov	eax, 0
		jmp short .remove_error_return
		
	.remove_error_interface:
		
	.remove_error_return:		
	popfd	;Enable Interrupts(if it were enabled before)
	pop	edi
	pop	esi
	pop	ecx
	pop	ebx
	stc
	ret
	

;===============================================================================

;mult.TL_scan	;(ebx = Entry Pointer, ecx = Entries Left) == (dx = TSS Selector)

.TL_scan:
;Scan the list for dx(= TSS selector) and return
;Used to find a specific Task
;Return:	ebx = pointer att found Entry
;		ecx = number of entries to the end of the list

	mov	ebx, multitasking_struc.Head
	mov	ecx, [ebx+multitasking_struc.last]		;Last Entry
	shr	ecx, multitasking_struc.Entry_Size_2
	mov	ebx, multitasking_struc.Base
	
	cmp	ecx, 0	;is the list empty?
	je	.TL_scan_NotFound
	
	.TL_scan_Loop:
	add	ebx, multitasking_struc.Entry_Size	;Jumps over the first entry = idle entry
	cmp	dx,	[ebx]
	je	.TL_scan_Found
	loopne .TL_scan_Loop

	.TL_scan_NotFound:
	stc		;Error: Entry not found
	ret

	.TL_scan_Found:
	dec	ecx
	clc
	ret

	