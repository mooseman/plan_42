;Setup Temporary PageDirectory
Temp:
	push	edx	;PT1 address
		call	check_edi
	pop	eax
	
	push	edi
		add	edi, 4
		add	eax, 3
		stosd
		mov	ecx, pages/4 - 2
		pt_fill 0

;Temporary PT0
		call	check_edi
	pop	edx
	push	edx
		mov	eax, edi
		add	eax, 3
		mov	[edx], eax

		;memory area where the processor stores the previous Task state when TR = 0
		push	edi
			add	edi, pages
			call	check_edi
			mov	eax, edi
		pop	edi
		add	eax, 3
		mov	[edi], eax

		;running code
		add  edi, (mem >> 12) * 4
		mov	ecx, Loader_Size
		mov  eax, (mem & 0FFFFF000h)+3
		.loop:
		stosd
		add	eax, pages
		loop	.loop
		
	pop	edx	;edx = Temp PD address