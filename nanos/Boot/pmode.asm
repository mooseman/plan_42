
	;Initiate Pmode
	;bx = seg pointer at nanos.dat

	%include 'A20.ASM'

	lgdt [gdtr]	;points at ds:gdtr	 ;A temporary GDT for initiation

	;Enable PMode
	mov  eax,cr0
	or   al, 1
	mov  cr0,eax		;Pmode Enabled

	;Selectors	
	mov  ax, 1 << 3	   ;1 = Data Selector
	mov  ds, ax
	mov	es, ax
	mov  ss, ax
	mov eax, stack_esp
	mov esp, eax

;PMode running on ds and es
	;dx = segment after module data
	
;jump to loaded code(Nanos, Loader.asm)
jmp dword (2 << 3):mem ;entering Pmode, 2 = Code Selector

