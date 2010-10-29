;
;	Nanos Header
;

%define Kernel 8:0

	desc.create_data	equ	20h		;(eax = Selector) == (edx [Settings][Selector], ecx Size)
	desc.delete		equ	21h		;(eax Status) == (edx Selector)
	desc.set			equ	25h		;() == (edx [Settings][Selector])
	page.alloc		equ	28h		;() == (edx = [Settings][Selector], ebx = Base, ecx = Size)
	
	module.create		equ	10h		;(eax Selector) == (ecx LDT Size)
	module.move		equ	14h		;(eax Selector) == (edx Source selector, ebx Target [module][selector])
	module.movetask	equ	15h		;() == (edx Source Task selector, ebx Target Module)
	
	mult.create		equ	30h		;(eax = Selector) == (edx EIP, ebx ss, ecx ESP)
	mult.add			equ	34h		;() == (dx = TSS Selector)
	mult.get			equ	32h		;() == (edx TSS Selector, ds:ebx TSS segment data)
	mult.set			equ	33h		;() == (edx TSS Selector, ds:ebx TSS segment data)
	
%if 1 = 0

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
	
%endif