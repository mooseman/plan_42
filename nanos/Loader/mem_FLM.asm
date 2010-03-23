;Free Linear Memory - FLM
	; ebx = linear address of free memory
	new_Page	lin_FLM
	
	mov	ecx, pages/4
	mov	eax, ebx		; ebx = linear address of free memory
	add	eax, pages	; include this page
	put
	mov	eax, lin_Mod_Data * pages
	put
	mov	eax, [lin_Mod_Data * pages + Loader_init.data_end]	;size of segment
	add	eax, lin_Mod_Data * pages	;eax = next free lin.page
	put
	mov	eax, 0FFFFF000h
	put
	pt_fill   0


