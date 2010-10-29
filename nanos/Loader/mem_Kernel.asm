;Memory move
;
;Kernel
MoveKernel:
	mov	edx, lin_kern
	mov	ecx, size_kern
	mov	esi, Kernel_Start

	.move:	
	new_Page edx
	push	ecx
		mov	ecx, pages / 4
		rep	movsd
	pop	ecx
	inc	edx
	loop	.move



