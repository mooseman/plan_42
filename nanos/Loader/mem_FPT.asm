;Free Page Table, FreePT

;First entry:
;	Free memory(bytes) = pointer to last page in list


FPT:
	mov	ebx, lin_FPT
	new_Page ebx
	
	;Determine pages to reserve
	mov	edx, [Nanos_init.MemSize]	;edx = Memory size in bytes
	mov	ecx, edx
	dec	ecx
	shr	ecx, 22
	inc	ecx		;ecx = Pages needed to store pages

	;Save FPT Position
	shl	ebx, 12
	mov	[Nanos_init.lin_FPT], ebx
	shr	ebx, 12
	shl	ecx, 12
	mov	[Nanos_init.lin_FPT_Size], ecx
	shr	ecx, 12
	
	push	edi

		mov	esi, 0		;last page pointer
	
		;find first free page
		push	ecx
			mov	eax, edi
			.find_free:
				add	eax, pages
				
				cmp	eax, mem
				je	.add_mem
				.check_free_bios:
				cmp	eax, 0A0000h
				je	.add_bios
				.find_next:
			loop .find_free
				
			jmp	.find_done
				.add_mem:
				mov	eax, Loader_Size_bytes + mem
				jmp	.check_free_bios
				
				.add_bios:
				mov	eax, 100000h
				jmp	.find_next
			.find_done:
		pop	ecx
				
		add	eax, pages	;one page for FLM
		;eax = first free page

		
		add	edi, 4	;second entry = first free page
		
		;Write pages from loader		
		push	eax
		push	ecx
			mov	eax, mem
			mov	ecx, Loader_Size
			.loader_pages:
				stosd
				add	eax, pages
				add	esi, 4	
			loop	.loader_pages
		pop	ecx
		pop	eax
		
					
		;Start writing table
		.write:
			stosd
			add	eax, pages
			add	esi, 4	
			
			;end of memory
			cmp	eax, edx
			jae	.done
			
			;beginning of bios memory
			cmp	eax, 0A0000h
			jne	.bios_done
			mov	eax, 100000h
			.bios_done:

			;beginning of loader memory
			;loader page is already in the list
			cmp	eax, mem
			jne	.loader_done
			mov	eax, mem+Loader_Size_bytes
			.loader_done:

			;new page?
			test	edi, 0FFFh
			jnz	.write
			
			;new page
			inc ebx
			new_Page ebx

		loop	.write
							
		.done
		;write
		
		;ecx = pages left
		
		mov	eax, edi

	pop	edi
	mov	[edi], esi
		mov	edi, eax
	
	shl	esi, 10
	Print .numPages
	PrintHex esi, 8

	;finish last pages
	jmp short .fin
	.finish:
	new_Page ebx
	.fin:
	
	push	ecx
		mov	ecx, pages
		sub	ecx, edi
		and	ecx, 0FFFh
		shr	ecx, 2
		
		mov	eax, 0
		rep	stosd
	pop	ecx
	
	inc ebx
	loop .finish
	
	
	shl	ebx, 12
	; ebx = linear address of free memory

	jmp	.over
	
	.numPages db 10,'Free memory: ',0
	
	.over:
	
	
jmp	debug_over
	;DEBUG==============================
debug:
pusha
	Print .debug_text
	PrintHex eax, 8
	PrintByte ' '
	PrintHex ebx, 8
	PrintByte ' '
	PrintHex ecx, 8
	PrintByte ' '
	PrintHex edi, 8
	PrintByte ' '

popa	

ret
.debug_text:	db 10,'eax      ebx      ecx      edi',10,0

;END DEBUG=========================
debug_over: