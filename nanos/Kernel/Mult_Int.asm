;Multitasking management

;occurs each timer interrupt


;Start
	push ds
	push eax
	push ebx
	push	edx
		;Restore PIC
	mov al, 20h	;EOI
	out 20h, al

	mov  ax, data_sel
	mov  ds, ax

	mov  ebx, multitasking_struc.Head
	mov	eax, [ebx+multitasking_struc.current]	;eax = current
	mov	edx, [ebx+multitasking_struc.last]	;edx = last entry
		
	cmp	edx, byte multitasking_struc.Entry_Size	;Check number of entries
	jbe	.one_task 	;One or zero tasks in the list = no task switch
	
	add	ebx, byte multitasking_struc.HeadSize
	add	ebx, eax

	str  [ebx + multitasking_struc.current]		;Save TR for the task we are leaving

	call	.NextTask
	
.one_task:

	;Check if float state owner
	str	ax			;get current TSS
	cmp	ax, [ebx + multitasking_struc.float]	;compare to current float-State owner
	jne	.dont_clear_TS
	clts
	.dont_clear_TS:
	
	;return to previous task
	pop	edx
	pop  ebx
	pop  eax
	pop  ds
	sti
	iret ;Return

	.NextTask
	;eax = current(no base)
	;edx = last(no base)
	
		;move to the next task
		xor	ebx, ebx
		cmp	eax, edx	;is it the last entry
		setae bl
		dec	ebx
		and	eax, ebx
		add	eax, multitasking_struc.Entry_Size	;eax = next entry
		
		;Write current task number
		mov	ebx, multitasking_struc.Head
		mov	[ebx+multitasking_struc.current], eax
		
		;Start the next task
		mov	ebx, multitasking_struc.Base
		add	ebx, eax			;ebx = current TSS selector
		jmp far [ebx+multitasking_struc.current - 4]
	ret
	


;Occurs each time a task tries to use floating-point operations or MMX, SSE...
;and the current task is not the current state owner

;7: device not available
.device_not_available:

	push ds
	push	edx
	push	ebx
	
		mov  dx, data_sel
		mov  ds, dx

		mov  ebx, multitasking_struc.Base
		mov	dx, [ebx + multitasking_struc.float]	;Current state owner
		
		call	desc.get_seg_base	;(ebx = Segment Base) == (dx = Selector)
		jc	.save_state
		add	ebx, task_struc.Float		;Floating-point state area(512 bytes)
		fxsave [ebx]

		.save_state:
		str	dx
		call	desc.get_seg_base	;(ebx = Segment Base) == (dx = Selector)
		jc	.dont_read_state	;error: TSS is illegal, shouldn't hapen
		add	ebx, task_struc.Float		;Floating-point state area(512 bytes)
		fxrstor [ebx]
		
		.dont_read_state:
	pop	ebx
	pop	edx
	pop  ds
	sti
	iret

;;;;;;;;;;;;;;;;;;;;