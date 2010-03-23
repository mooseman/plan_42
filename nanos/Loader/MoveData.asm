;Creates the memory structure in nanos
;
;edi points to 0 in the beginning(physical memory)
;edi is increased by all parts and left for the next part to continue
;call check_edi is to be called between every page


	jmp  mem_data_over

%include 'mem_data.asm'

check_edi:
	;Even pages check:
		mov  eax, edi
		and  eax, 0FFFh
		jz	.bios
		.eternal:
		hlt
		jmp	.eternal
	.bios:
	cmp	edi, mem
	je	.fix_loader
	cmp	edi, 0A0000h
	je	.fix_bios
		
	PrintHex edi, 8
	PrintByte ' '
	ret

	.fix_loader:
		add	edi, Loader_Size_bytes
		jmp	.bios
		
	.fix_bios:
		mov	edi, 100000h
		jmp	.bios

		
deb	db 10,'Data:    Address:',10,0	
deb1	db 10,'PD:      ',0
deb2	db 10,'PT1:     ',0
deb3	db 10,'PT2:     ',0
deb4	db 10,'Kernel:  ',0
deb5	db 10,'Module:  ',0
deb6	db 10,'Stack:   ',0
deb7	db 10,'GDT:     ',0
deb8	db 10,'IDT:     ',0
deb9	db 10,'TL:      ',0
deba	db 10,'FPT:     ',0
debb	db 10,'FLM:     ',0
debc	db 10,'Temp PD: ',0


mem_data_over:

push	ebx	;ebx = nanos.dat

	Print deb
	
;Memory Layout
;	page:		contents:
;	0		BIOS data
;	1		free(boot code: 10)
;	20-x		Loader
;	x-A0		Free
;	A0-100	BIOS(Video, Code)
;	100-		Free


	;Start of memory initiation
	;Page 1: PD - Page Directory
	Print deb1
		mov  edi, mem_PD * pages		;Keeps track of where to write througout this initiation
		call	check_edi
		%include 'mem_PD.asm'	

		
	;Page x: PT1 - Page Table 1
	Print deb2
		new_Page 1
pop	ebx	;ebx = nanos.dat
push	edi
push	ebx	;ebx = nanos.dat

		%include 'mem_PT1.asm'

	Print deb3
		new_Page 2
		%include 'mem_PT2.asm'

	;BIOS data + Nanos System Data
	push	edi
		mov	eax, lin_BIOS * pages	;eax = linear address
		mov	edi, 0				;edi = physical address
		call	PD_add_Page
	pop	edi
				
	;Move kernel
	Print deb4
		%include 'mem_kernel.asm'
		

pop	ebx	;ebx = nanos.dat

	;Move Loader module
	Print deb5
		%include 'mem_module.asm'

	;Kernel idle Stack
	Print deb6
		%include 'mem_stack.asm'
	
	;GDT
	Print deb7
		%include 'mem_GDT.asm'

	;IDT
	Print deb8
		%include 'mem_IDT.asm'

	;Task list
	Print deb9
		%include 'mem_TL.asm'

	;Free Page
	Print deba
		%include 'mem_FPT.asm'
		; ebx = linear address of free memory
	;one page is reserved for Free Linear memory
		
	;Free Linear
		; ebx = linear address of free memory
	Print debb
		%include 'mem_FLM.asm'

pop	edx	;edx = physical address for PT1

	;Temp PD and PT1
	Print debc
		%include 'Temp_PD.asm'
		;edx = Temp PD address

;edi is out of sync in Temp_PD.asm
;Fix it before writing more here
;else nothing has to be done
;check_edi

;Done

		;edx = Temp PD address