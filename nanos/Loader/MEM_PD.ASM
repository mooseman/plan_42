;Setup Pageing

    ;PD - Page Directory
	mov	ecx, pages / 4
	mov	eax, edi
	and	eax, 0FFFFF000h
	or	eax, 3
	put
	
	pt_fill 0
	
	jmp	PD_over
	
PD_add_Page:
	;Map linear address to physical address
	;eax = linear address
	;edi = physical address

	push	eax
	
		;Step 1: PD
		and	eax, 0FFC00000h
		shr	eax, 20
		add	eax, mem_PD * pages
		mov	ebx, [eax]
		and	ebx, 0FFFFF000h
		;ebx = Pointer at PTx
		
	pop	eax

	;Step 2: PTx
	and	eax, 003FF000h
	shr	eax, 10
	add	ebx, eax
	;ebx = pointer att page entry

	or	edi,         1	;Present
	mov	[ebx], edi
	and	edi, 0FFFFF000h
	ret
	
PD_over: