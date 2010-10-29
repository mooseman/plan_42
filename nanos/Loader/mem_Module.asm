;Memory move
;
;Loader module


	;ebx = nanos.dat

Load_mod:
	mov	edx, lin_Mod_Data	;page
	
	xor	ecx, ecx
	mov	cx, [ebx + Loader_init.data_end]
	shl	ecx, 4
	sub	ecx, mem	;ecx = size in bytes(even sectors, 512 byte)
	dec	ecx
	shr	ecx, 12
	push	ecx
		inc	ecx		;ecx = size in pages
	
		shl	ecx, 12
		mov	[ebx + Loader_init.data_end], ecx
		shr	ecx, 12
		
		mov	esi, ebx

		.move:	
		new_user_Page edx
		push	ecx
			mov	ecx, pages / 4
			rep	movsd
		pop	ecx
		inc	edx
		loop	.move
		
	;Stack
		new_user_Page lin_Mod_Stack

		put_fill   0, 0, pages
		
	;LDT
		new_Page lin_Mod_LDT

		;Module data(name)
		put_fill	mem_data_Mod_size, mem_data_Mod, module_struc.LDT
		
		;Module LDT
		put_fill	mem_data_Mod_LDT_size, mem_data_Mod_LDT, pages - module_struc.LDT	

	pop	ecx	;ecx = size in pages - 1	
	;fix limits of code and data segments
	mov	ebx, edi
	add	ebx, module_struc.LDT - pages
	mov	[ebx], cx		;Code segment
	mov	[ebx+8], cx	;Data segment
	

;TSS
	new_Page lin_Mod_TSS

	put_fill	mem_data_Mod_TSS_size, mem_data_Mod_TSS, pages
	