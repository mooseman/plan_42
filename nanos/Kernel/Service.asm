;===========================================
; Services
;
; Converts C calling to kernel service
; Preserve: EBP, EBX, ESI, EDI
;
; Stack:
;	ebp +  0	ebp
;	ebp +  4	eip	
;	ebp +  8	cs
;	ebp +  C	Service number	
;	ebp + 10	param 1

%define	old_ebp		ss:ebp
%define	caller_ds		ss:ebp + 4
%define	caller_eip	ss:ebp + 8	;Callers EIP
%define	caller_cs 	ss:ebp + 12	;Callers CS


service:
	push	ds
	push	ebp
	mov	ebp, esp
		
	;fix ds
	push	data_sel
	pop	ds

	;eax = service number
	and	eax, 03Fh		;40h services
	shl	eax, 2
	add	eax, .list
	call	[cs:eax]
		
	pop	ebp
	pop	ds
	retf
	
.no_service:
	;wrong Service number
	mov	eax, err_no_service
	stc
	ret
	
align 4
		
.list: ;Service number
;00		;OS specific
	dd	sys.version		;(eax = Version) == ()
	dd		.no_service
	dd		.no_service
	dd		.no_service
	dd	page.freemem		;(eax Free memory) == ()
	dd		.no_service
	dd		.no_service
	dd		.no_service
;08		System services
	dd		.no_service
	dd		.no_service
	dd		.no_service
	dd		.no_service	;connect IRQ to interface
	dd		.no_service	;DMA
	dd		.no_service	;PIC/APIC
	dd		.no_service
	dd		.no_service
;10		Module management
	dd	module.create		;(eax Selector) == (ecx LDT Size)
	dd	module.delete		;() == (edx Selector)
	dd		.no_service	;get settings
	dd		.no_service	;set settings
	dd	module.move		;(eax Selector) == (edx Source selector, ebx Target [module][selector])
	dd	module.movetask	;() == (edx Source Task selector, ebx Target Module)
	dd		.no_service	
	dd		.no_service
;18 +	Unknown
	dd		.no_service
	dd	life	;return meaning of life in eax
	dd		.no_service
	dd		.no_service
	dd		.no_service
	dd		.no_service
	dd		.no_service
	dd		.no_service
;20 +	Descriptors, Linear memory
	dd	desc.create_data		;(eax = Selector) == (edx [Settings][Selector], ecx Size)
	dd	desc.delete			;(eax Status) == (edx Selector)
	dd		.no_service
	dd		.no_service
	dd	desc.get				;(eax [Settings][Selector]) == (edx Selector)
	dd	desc.set				;() == (edx [Settings][Selector])
	dd		.no_service
	dd		.no_service
;28 +	Paging, Physical memory
	dd	page.alloc		;() == (edx = [Settings][Selector], ebx = Base, ecx = Size)
	dd	page.dealloc		;() == (edx = Selector, ebx = Base, ecx = Size)
	dd		.no_service
	dd		.no_service
	dd		.no_service	;get phys address
	dd		.no_service
	dd	page.phys_allocate		;() == (edx Selector, ebx Base, edi First PTE, ecx Size)
	dd		.no_service	;
;30 +	Multitasking
	dd	mult.create		;(eax = Selector) == ()
	dd	mult.delete		;() == (edx Selector)
	dd	mult.get			;() == (edx TSS Selector, ds:ebx TSS segment data)
	dd	mult.set			;() == (edx TSS Selector, ds:ebx TSS segment data)
	dd	mult.add			;() == (dx = TSS Selector)
	dd	mult.remove		;(eax Status) == (edx = TSS Selector)
	dd		.no_service
	dd		.no_service	
;38 +	Interface
	dd	interface.create	;() == (edx Type, ebx Process(TSS selector))
	dd	interface.delete	;() == (edx Type)
	dd	interface.call		;() == (edx Type)
	dd		.no_service	;Get param
	dd	interface.connect	;() == (edx Type, ebx [source][target] module)
	dd		.no_service	;Disconnect interface
	dd		.no_service
	dd		.no_service	;Abort interface
	
.list_end:

	;.list must be 256 bytes
	.list_size equ 100h
	times (.list_end-.list) - .list_size	dd 0	;List is too small
	times .list_size - (.list_end-.list)	dd 0 ;List is too big

%include 'sys.asm'
%include 'sysm.asm'
%include 'desc.asm'
%include 'page.asm'
%include 'mult.asm'
%include 'interface.asm'
%include 'module.asm'


life:
	mov	eax, 4242424242
	ret