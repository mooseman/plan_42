;IDT
	new_Page lin_IDT

	put_fill   mem_data_IDT_size, mem_data_IDT, 100h * 8	;100h descriptors
	add  edi, 800h



; desc  offset, selector, control   ;For gate descriptors

;Each descriptor should have exactly one of next 8 codes to define the type of
;descriptor
;D_TASK		EQU	 500h	;Task gate
;D_INT		EQU	0E00h	;386 interrupt gate
;D_TRAP		EQU	0F00h	;386 trap gate

;Descriptors may include the following as appropriate:
;D_DPL3		EQU	6000h	;DPL3 or mask for DPL
;D_DPL2		EQU	4000h
;D_DPL1		EQU	2000h
;D_PRESENT	EQU	8000h	;Present
;D_NOT_PRESENT	EQU	8000h	;Not Present
				;Note, the PRESENT bit is set by default
				;Include NOT_PRESENT to turn it off
				;Do not specify D_PRESENT

;Segment descriptors (not gates) may include:

;D_BIG		EQU	  40h	;Default to 32 bit mode (USE32)
;D_BIG_LIM	EQU	  80h	;Limit is in 4K units



;Reference:
;0   Divide Error
;1   Debug Exception
;2   NMI  Interrupt
;3   Breakpoint
;4   INTO - overflow
;5   BOUND Range exceeded
;6   Invalid opcode
;7   Device not available
;8   Double Fault
;9   Co-processor segment overrun
;A   Invalid TSS
;B   Segment Not Present
;C   Stack Fault
;D   General Protection Fault
;E   Page Fault
;F
;10  Floating Point Error
;11  Alignment Check
;...
;1E
;1F  Reserved

;20  IRQ 0
;21  IRQ 1
;.
;.
;.
;2F  IRQ F

;30  Free
;.
;.
