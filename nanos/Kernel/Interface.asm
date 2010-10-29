;
; Interface management
;
;
interface:

;===============================================================================
;
;interface.create	;() == (edx Type, ebx Process(TSS selector))

;	Create an In-interface

.create:
	push	ecx
	push	ebx
	push	edx
		
		call	interface.get_interface_base	;(ebx Base) == ()
		mov	ecx, module_struc.InterfaceCount
	
		or	edx, interface_struc.Type_present	;set present bit
	
		.scan:
			cmp	[ebx + interface_struc.Type], edx
			je	.create_error
			bts dword	[ebx + interface_struc.Type], interface_struc.Type_present
			jnc	.found_free
			add	ebx, module_struc.InterfaceEntSize
		loop	.scan
		jmp	.create_error	;found no free entries
				
		;Create new
		.found_free:
		
		;Write Type
		mov	[ebx + interface_struc.Type], edx
		
		;Test if out-type
		bt	edx, interface_struc.Type_in_interface
		jnc	.create_out
		
		mov	edx, [ss:esp + 4]	;TSS selector
		
		push	ebx
	
			;Is running?
			call	mult.TL_scan	;(ebx = Entry Pointer, ecx = Entries Left) == (dx = TSS Selector)
			jnc	.create_error_int
		
			;check TSS
			lar	ecx, edx
			jnz	.create_error_int	;not found, error
			and	ecx, 00109F00h		;		Busy: 0
			cmp	ecx, 00008900h		;		Interface: 0	(Avail bit)
			jne	.create_error_int
			
			
			call	desc.get_desc_base	;(ebx = Base) == (dx = selector)
			bts dword [ebx+4], 14h	;Set Interface bit

		pop	ebx
					
		jmp	.create_in
				
		.create_out:
		xor	edx, edx
		.create_in:
		mov	[ebx + interface_struc.TSS], dx
		.create_in_out:
		xor	edx, edx
		mov	[ebx + interface_struc.Module], dx	;Write Connected module/Calling TSS(= 0)
		
	pop	edx
	pop	ebx
	pop	ecx
	clc
	ret
	
		
		.create_error_int:	;TSS error, not a TSS or "busy" TSS
		pop	ebx
		;Clear type field in interface
		xor	edx, edx
		mov	[ebx + interface_struc.Type], edx
	
	.create_error:
	pop	edx
	pop	ebx
	pop	ecx
	stc
	ret
		
;===============================================================================
;
;	interface.delete		;() == (edx Type)
;
;	Delete an interface

.delete:
	push	ebx
	push	eax
		call	interface.get_interface_base	;(ebx Base) == ()
	
		;scan for type(edx)
		.delete_scan:
			cmp	[ebx + interface_struc.Type], edx
			je	.delete_found
			add	ebx, module_struc.InterfaceEntSize
		loop	.delete_scan
		jmp	.delete_error	;found no free entries
		
		.delete_found:
		;Called from .delete_task
		;[ebx] = Interface entry
		
		;Disconnect interface
		push	ebx
			sldt	bx
			call	interface.disconnect		;() == (edx Type, ebx module)
		pop	ebx				
		
		;Remove Interface
		%if module_struc.InterfaceEntSize != 8
			%error "InterfaceEntSize changed"
		%endif
		xor	eax, eax
		mov	[ebx], eax
		mov	[ebx+4], eax
		
	pop	eax
	pop	ebx
	ret
	
	.delete_error:
	pop	eax
	pop	ebx
	stc
	ret


;===============================================================================
;
;	interface.delete_task		;() == (edx Task)
;
;	Delete an interface, given a task

.delete_task:
	push	ebx
	push	eax
		call	interface.get_interface_base	;(ebx Base) == ()
	
		;scan for type(edx)
		.delete_task_scan:
			bt dword [ebx + interface_struc.Type], interface_struc.Type_in_interface
			jnc	.delete_task_scan_next
			cmp	[ebx + interface_struc.TSS], dx
			je	.delete_task_found
			.delete_task_scan_next:
			add	ebx, module_struc.InterfaceEntSize
		loop	.delete_task_scan
		jmp	.delete_error	;found no free entries
		
		.delete_task_found:
		;[ebx] = Interface entry
		mov	edx, [ebx + interface_struc.Type]
		jmp	.delete_found


;===============================================================================
;
;	interface.connect		;() == (edx Type, ebx [source][target] module)
;
;	Makes an connection of type, from source module to target module
;
.connect:
	pusha
		mov	ecx, ebx
		
		;Disconnect source interface
		mov	ebx, ecx
		shr	ebx, 10h
		and	edx, ~(interface_struc.Type_in_interface)
		call	interface.disconnect		;() == (edx Type, ebx module)
		jc near .connect_error
			
		;Disconnect target interface
		mov	ebx, ecx
		and	ebx, 0FFFFh
		or	edx, interface_struc.Type_in_interface
		call	interface.disconnect		;() == (edx Type, ebx module)
		jc near .connect_error
		
			
		;Get Source Interface Base
		mov	edx, ecx
		shr	edx, 10h
		call	desc.get_ldt_base	;(ebx = Base) == (edx = Selector)
		jc near .connect_error	;LDT error
		add	ebx, module_struc.Interface - module_struc.LDT
		mov	esi, ebx
	
		;Get Target Interface Base
		mov	edx, ecx
		and	ebx, 0FFFFh
		call	desc.get_ldt_base	;(ebx = Base) == (edx = Selector)
		jc near .connect_error	;LDT error
		add	ebx, module_struc.Interface - module_struc.LDT
		mov	edi, ebx
		
		
		;Scan Source for edx
		mov	ebx, esi
		mov	ecx, module_struc.InterfaceCount
		and	edx, ~(interface_struc.Type_in_interface)
		.connect_source_scan:
			cmp	[ebx + interface_struc.Type], edx
			je	.connect_source_found
			add	ebx, module_struc.InterfaceEntSize
		loop	.connect_source_scan
		jmp	.connect_error	;found no free entries
		.connect_source_found:
		mov	esi, ebx		

		;Scan Target for edx
		mov	ebx, edi
		mov	ecx, module_struc.InterfaceCount
		or	edx, interface_struc.Type_in_interface
		.connect_target_scan:
			cmp	[ebx + interface_struc.Type], edx
			je	.connect_target_found
			add	ebx, module_struc.InterfaceEntSize
		loop	.connect_target_scan
		jmp	.connect_error	;found no free entries
		.connect_target_found:
		mov	edi, ebx

		;Write interface data:
		;Target:
		;	.Module = source module
		;Source:
		;	.TSS = target TSS
		shr	ecx, 10h
		and	ecx, 0FFF8h	;source module
		mov	[ds:edi + interface_struc.Module], cx
		mov	dx, [ds:edi + interface_struc.TSS]
		mov	[ds:esi + interface_struc.TSS], dx

	popa
	clc		
	ret

	
	.connect_error:
	popa
	stc
	ret
;===============================================================================
;
;	interface.disconnect		;() == (edx Type, ebx module)
;	Removes an connection of type from module
.disconnect:
	
	ret

;===============================================================================
;
;	interface.call		;() == (edx Type)
;
;	Call an interface
;
.call:
	push	ebx
	push	ecx
		call	interface.get_interface_base	;(ebx Base) == ()
		
		;Scan for edx
		mov	ecx, module_struc.InterfaceCount
		or	edx, interface_struc.Type_in_interface
		.call_scan:
			cmp	[ebx + interface_struc.Type], edx
			je	.call_found
			add	ebx, module_struc.InterfaceEntSize
		loop	.call_scan
		jmp	.call_error	;found no matching entry
		.call_found:

		;Write Calling TSS
		str	[ebx + interface_struc.Caller]
		
		;Make the call
		call	far [ebx + interface_struc.TSS]
		
		;Write Calling TSS
		mov word [ebx + interface_struc.Caller], 0
		
	pop	ecx
	pop	ebx
	clc
	ret
	
	.call_error:
	pop	ecx
	pop	ebx
	stc
	ret
	
;===============================================================================
;
;interface.get_param		;(eax Caller) == (edx Interface_Type)
;	Get interface status
;	In:
;		int	Interface ID
;	Return:	int	Status
.get_param:
;Read Status
;Return:	Caller(if any)


;===============================================================================
;
;interface.abort		;(eax Caller) == (edx Interface_Type)
;	Abort an active interface connection
;	In:
;		int	Interface ID
;	Return:	int	Status
.abort:
;Read Status
;Return:	Caller(if any)
	ret

	
	
	
;===============================================================================
;
;	interface.get_interface_base	;(ebx Base) == ()
;
.get_interface_base:
	push	edx
		;Get LDT Base
		sldt	dx
		call	desc.get_ldt_base	;(ebx = Base) == (dx = Selector)
		jc	.get_base_error	;LDT error???
		
		;Get Interface Base
		add	ebx, module_struc.Interface - module_struc.LDT
	pop	edx
	
	.get_base_error
	pop	edx
	stc
	ret
	
	
	
