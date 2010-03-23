sys:

;===============================================================================

;sys.version	;(eax = Version) == ()

.version:
    mov  eax, nanos_version
    clc
    ret

    

;===============================================================================
;
;system.interrupt	;(eax = Interrupt) == (int Interrupt, int Selector, int Offset)
;	Create a Interrupt Gate

; .interrupt
; 	push edx
; 	
; 		;Check Privileges
; 		mov	ax, [caller_cs]
;		movzx edx, word [param2]
; 		arpl	dx, ax
; 		
; 		lar	edx, edx
; 		jnz	.interrupt_error		;Privilege check failed
; 		shr	edx, 8
; 		and	edx, 18h
; 		cmp	edx, 18h
; 		jne	.interrupt_error		;Not a Code segment
;
; 		;Check Interrupt
;		mov	edx, [param1]
; 		and	edx, 0FFh
; 		shl	edx, 3
; 		add	edx, lin_IDT * pages
; 		bts dword [edx + 4], 15
; 		jc	.interrupt_error		;Interrupt already taken
;
; 		;Write descriptor
; 		;Int.Gate    [Offset      16]1Pl0S110[------][Selector      ][Offset       0]
;		mov	eax, [param3]
; 		mov	[edx], ax
; 		shr	eax, 10h
; 		mov	[edx + 6], ax
; 		mov	eax, 8E000000h
;		mov	ax, [param2]
; 		mov	[edx + 2], eax
;
; 		;Done
;		mov	eax, [param1]
; 		and	eax, 0FFh
; 	pop	edx
; 	clc
; 	ret
; 						
; 	.interrupt_error:
; 	xor	eax, eax
; 	pop	edx
; 	stc
; 	ret	

	
;===============================================================================
;
;system.delete_interrupt	;(eax = Error code) == (int Interrupt)
;	Delete a Interrupt Gate
;

; .delete_interrupt
; 	push edx
; 	
; 		;Check Privileges
; 		mov	ax, [caller_cs]
; 		mov	dx, 0
; 		arpl	ax, dx				;only privilege 0 may remove a interrupt gate
; 		jz	.delete_interrupt_error
; 		
; 		;Remove interrupt
; 		mov	edx, [param1]
; 		and	edx, 0FFh
; 		shl	edx, 3
; 		add	edx, lin_IDT * pages

; 		;Write descriptor
; 		mov	[edx], dword 0
; 		mov	[edx + 4], dword 0

; 		;Done
; 		mov	eax, 0
; 	pop	edx
; 	clc
; 	ret
; 						
; 	.delete_interrupt_error:
; 	mov	eax, -1
; 	pop	edx
; 	stc
; 	ret	
