sysm:

	
	
;===============================================================================
;
;	sysm.create_callgate	;(eax = Selector) == (int CallG selector, int Target selector, int offset, int DWord count)
;	Create a CallGate
;
;	In:
;		param1	Selector for CallGate
;		param2	Selector for Target
;		param3	Offset
;		param4	Dword count
;	Out:
;		int		Selector

;.create_callgate:
; 	push	ecx
; 	push edx
; 	
; 		;Check Privileges
; 		mov	ax, [caller_cs]
; 		arpl	[param1], ax
; 		arpl	[param2], ax
; 		jz	.call_error	;Error: DPL < CPL
; 		
; 		lar	edx, edx
; 		jnz	.call_error		;Privilege check failed
; 		shr	edx, 8
; 		and	edx, 18h
; 		cmp	edx, 10h
; 		jne	.call_error		;Not a Code segment

; 		mov	eax, [param1]
; 		mov	edx, [param3]
; 		mov	ecx, [param4]
; 		and	ecx, 1Fh
; 		or	ecx, 8C00h
; 		shl	ecx, 10h
; 		mov	cx, [param2]
; 		call	.gate	;(same, eax = selector)==(eax = descriptor selector, edx = offset, ecx = settings+target selector)
; 					;CallGate    [Offset      16]1Pl0S100[-][Dwc][Selector      ][Offset       0]
; 		jc	.call_error
; 	
; 	pop	edx
; 	pop	ecx
; 	clc
; 	ret
; 						
; 	.call_error:
; 	xor	eax, eax
; 	pop	edx
; 	pop	ecx
; 	stc
; 	ret		
		

;===============================================================================
;
;	sysm.gate	;(same, eax = selector)==(eax = descriptor selector, edx = offset, ecx = settings+target selector)
;TaskGate    [--------------]1Pl00101[------][Selector      ][--------------]
;CallGate    [Offset      16]1Pl0S100[-][Dwc][Selector      ][Offset       0]
;Int.Gate    [Offset      16]1Pl0S110[------][Selector      ][Offset       0]
;TrapGate    [Offset      16]1Pl0S111[------][Selector      ][Offset       0]

.gate
	push	ecx
	push	edx
		
		rol	ecx, 3
		arpl cx, ax
		ror	ecx, 3
		and	ecx, 0EF1FFFFFh
		or	ecx,  84000000h
		
		;Create Descriptor
		
		call	desc.create_desc	;(eax = selector, ebx = Desc Base) == (eax = selector)
		jc	.gate_error

		;Write data to descriptor
		mov	[ebx + 2], ecx		;Selector, settings, Dword(CallGate)
		mov	[ebx], dx			;Offset
		shr	edx, 10h
		mov	[ebx + 6], dx
			
		;Done
		
	pop	edx
	pop	ecx
	clc
	ret
	
	.gate_error:
	pop	edx
	pop	ecx
	stc
	ret
	
		
