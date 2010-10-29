	%define USERERRORS ERROR!

	%define BSTS "Better Safe Than Sorry!"
;This only checks actions that are compile-static
;If no errors occurs whith this on, none should occur with it off.

%include 'extern.asm'
%include '../mem.asm'
%include '../const.asm'
%include '../struc.asm'
%include 'macro.asm'
%include 'gdt.inc'

[BITS 32]

SEGMENT Loader

start:
	;bx = nanos.dat segment
	shl	ebx, 4
	and	ebx, 000FFFFFh
	push	ebx	;ebx = nanos.dat
	
		;Initiate RS232/VGA interface
		%include 'rs232vga.asm'
		
		;Welcome message
		Print Welcome	

		;Determine size of RAM above 1MB	
		Print RAMSize
		
		;"Zero" RAM
		mov	ebx,  12345678h
		mov	edx, 0e0000000h
		mov	ecx, 1000h	;max tries
		.zero:
		mov	[edx], ebx
		add	edx, 100000h
		loop	.zero

		;Check RAM
		mov	edx, 0
		mov	eax, 5A5A5A5Ah	;test dword
		
		mov	ecx, 1000h	;max tries
		.test:
			;test memory
			cmp	[edx], ebx
			jne	.eom
			mov	[edx], eax
			cmp	[edx], eax
			jne	.eom
			not	eax
			mov	[edx], eax
			cmp	[edx], eax
			jne	.eom
			not	eax
			;Continue
			add	edx, 100000h	;1MB steps
		loop	.test

		.eom:
		PrintHex edx, 8
		PrintByte 10	
		;edx = amount of memory in bytes
		mov	[Nanos_init.MemSize], edx
			
			
		;Fix pic
		%include 'pic.asm' ;IF cleared

	pop	ebx	;ebx = nanos.dat	

;Move Data and make tables, edx = RAM size
%include 'movedata.asm'
	;edx = Temp PD address
	push	edx
	
		Print Starting
		
	pop	edx
	;edx = Temp PD address
;Load PDBR with temporary PD
	mov  eax, cr3  ;PDBR
	and  eax, 0FFFh ;keep settings
	or   eax, edx
	mov  cr3, eax

;Enable Paging
	mov  eax, cr0
	or   eax, 80000000h ;PG set
	mov  cr0, eax

;Load Nanos GDT, IDT

	;IDT
	mov  ebx, nanos_idtr
	lidt [ebx]	;remake

	;GDT
	mov  ebx, nanos_gdtr
	lgdt [ebx]	;remake

	jmp  Mod_TSS_sel: 00000000h		;Bochs loads the new settings, Intel saves current state before


;Nanos table registers
	;IDT reg
nanos_idtr:	dw 100h*8-1  		; IDT Limit = 256 descriptors
	.base:	dd lin_IDT * pages	; IDT Base

	;GDT reg
nanos_gdtr:	dw 1000h-1		; GDT Limit = 1page
	.base:	dd lin_GDT * pages	; GDT Base



Welcome	db 'Initiating Nanos ver 0.04 by Peter Hultqvist',10,'http://www.neocoder.net/nanos/',10,0
RAMSize	db 'Counting RAM: ',0
Starting	db 10,10,'     Starting Nanos...',0
sok:	;start of kernel

