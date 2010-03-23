;
;
;Nanos System Library
;
; C Library
;

[BITS 32]

SECTION .text

global start
extern _main
start:
	;create interfaces
	
	call	_main
	
	;end process
	jmp	$

%include '../mem.asm'

;=============================
; service macro:
;
; service name, number, [parameters...]
;=============================
;     orig_ebp ss:ebp 
;   return_eip ss:ebp + 4
%define param1 ss:ebp + 8
%define param2 ss:ebp + 0Ch
%define param3 ss:ebp + 10h
%define param4 ss:ebp + 14h
%define param5 ss:ebp + 18h
%define param6 ss:ebp + 1Ch


%macro service 2
	global _%1
	_%1:
	mov	eax, %2
	call	kern_sel:00000000h
	ret
%endmacro


	
%macro service 3	;one parameter
	global _%1
	_%1:
	push	ebp
		mov	ebp, esp
		push	%3
			mov	%3, [param1]
			mov	eax, %2
			call	kern_sel:00000000h
		pop	%3
	pop	ebp
	ret
%endmacro

%macro service 4	;two parameters
	global _%1
	_%1:
	push	ebp
		mov	ebp, esp
		push	%3
		push	%4
			mov	%3, [param1]
			mov	%4, [param2]			
			mov	eax, %2
			call	kern_sel:00000000h
		pop	%4
		pop	%3
	pop	ebp
	ret
%endmacro

%macro service 5	;3 parameters
	global _%1
	_%1:
	push	ebp
		mov	ebp, esp
		push	%3
		push	%4
		push	%5
			mov	%3, [param1]
			mov	%4, [param2]			
			mov	%5, [param3]			
			mov	eax, %2
			call	kern_sel:00000000h
		pop	%5
		pop	%4
		pop	%3
	pop	ebp
	ret
%endmacro

%macro service 6	;4 parameters
	global _%1
	_%1:
	push	ebp
		mov	ebp, esp
		push	%3
		push	%4
		push	%5
		push	%6
			mov	%3, [param1]
			mov	%4, [param2]			
			mov	%5, [param3]			
			mov	%6, [param3]			
			mov	eax, %2
			call	kern_sel:00000000h
		pop	%6
		pop	%5
		pop	%4
		pop	%3
	pop	ebp
	ret
%endmacro

;=============================
; The functions
;=============================


;	sys.version		;(eax = Version) == ()
service version, 1
;	page.freemem		;(eax Free memory) == ()
service memfree, 8

;	module.create		;(eax Selector) == (ecx LDT Size)
service module_create, 10h, ecx	
;	module.move		;(eax Selector) == (edx Source selector, ebx Target [module][selector])
service module_move, 14h, edx, ebx

service life, 19h

;	desc.create_data		(eax = Selector) == (edx [Settings][Selector], ecx Size)
service	data_segment, 20h, edx, ecx
;	sysm.delete		;(eax = Status) == (edx Selector)
service	delete_segment, 27h, edx

;	page.alloc		;() == (edx = [Settings][Selector], ebx = Base, ecx = Size)
service page_alloc, 28h, edx, ebx, ecx
;	page.dealloc		;() == (edx = Selector, ebx = Base, ecx = Size)
service page_free, 29h, edx, ebx, ecx
;	page.phys_allocate		;() == (edx Selector, ebx Base, edi First PTE, ecx Size)
service map_memory, 2Eh, edx, ebx, edi, ecx
;	mult.create		;(eax = Selector) == (edx EIP, ebx ss, ecx ESP)
service new_task, 30h, edx, ebx, ecx
;	mult.add		;() == (dx = TSS Selector)
service start_task, 32h, edx
;	mult.remove	;(eax Status) == (edx = TSS Selector)
service stop_task, 33h, edx
;	interface.create	;(eax = Status) == (edx Type, ebx Process(TSS selector))
service interface, 38h, edx, ebx
