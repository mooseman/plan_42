
;===================
mem_data_GDT:
    dd   0, 0														;Null Descriptor
    desc service, kern_sel, D_CALL + D_DPL3 + 0					;Call Gate(Kernel Services) + copy 0 dword
    desc lin_kern << 12, size_kern-1, D_CODE + D_READ + D_BIG + D_BIG_LIM		;Kernel Code
    desc 0, 0FFFFFh, D_DATA + D_WRITE + D_BIG + D_BIG_LIM					;Data(all 4GB)
    desc (lin_kern << 12) + Multitasking.tss, 68h - 1, D_TSS 				;Idle TSS
    desc (lin_Stack << 12), size_Stack - 1, D_DATA + D_WRITE + D_BIG + D_BIG_LIM		;Data, Stack
    desc (lin_Mod_LDT << 12) + module_struc.LDT, pages - module_struc.LDT - 1, D_LDT	;Mod LDT
    desc (lin_Mod_TSS << 12) + task_struc.TSS, 68h - 1, D_TSS			;Mod TSS    
mem_data_GDT_size equ $-mem_data_GDT

;===================
mem_data_IDT:

;Interrupt 0-1F    - Processor Exceptions
    desc Interrupt.exception00, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception01, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception02, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception03, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception04, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception05, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception06, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Multitasking.device_not_available, kern_sel, D_INT + D_DPL0 + D_BIG
    
    desc Interrupt.exception08, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception09, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception0A, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception0B, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception0C, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception0D, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception0E, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception0F, kern_sel, D_INT + D_DPL0 + D_BIG

    desc Interrupt.exception10, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception11, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception12, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception13, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception14, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception15, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception16, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception17, kern_sel, D_INT + D_DPL0 + D_BIG

    desc Interrupt.exception18, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception19, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception1A, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception1B, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception1C, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception1D, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception1E, kern_sel, D_INT + D_DPL0 + D_BIG
    desc Interrupt.exception1F, kern_sel, D_INT + D_DPL0 + D_BIG

;Interrupt 20-2F   - IRQ 0-F

    desc Multitasking.interrupt, kern_sel, D_INT + D_DPL0 + D_BIG

mem_data_IDT_size equ $-mem_data_IDT

;===================
mem_data_TL:
	
    dd	4	;Address to current TSS
    dd	4	;Address to last TSS, Limit of List, Size of list, 0 = only idle task
    dw	0	;Current task state owner(TSS selector)

times multitasking_struc.HeadSize-($-mem_data_TL) db 0

;===Entry 0: Idle task	
	dw	Idle_TSS_sel
	dw	Idle_TSS_sel
;Module task
	dw	Mod_TSS_sel
	dw	Mod_TSS_sel
    
mem_data_TL_size equ $-mem_data_TL


;===================
mem_data_Mod:
	db	'Loader Module v1'
mem_data_Mod_size equ $-mem_data_Mod

mem_data_Mod_LDT:
	mem_data_Mod_LDT.code_sel:
		desc lin_Mod_Data << 12, 0,	D_DPL1 + D_CODE + D_READ + D_BIG + D_BIG_LIM		;Code
	mem_data_Mod_LDT.data_sel:
		desc lin_Mod_Data << 12, 0,	D_DPL1 + D_DATA + D_WRITE + D_BIG + D_BIG_LIM	;Data
	mem_data_Mod_LDT.stack:
		desc lin_Mod_Stack << 12, size_Mod_Stack - 1,	D_DPL1 + D_DATA + D_WRITE + D_BIG + D_BIG_LIM	;Data, Stack
	mem_data_Mod_LDT.tss_stack:
		desc (lin_Mod_TSS << 12) + task_struc.Stack, task_struc.StackSize - 1,	D_DPL0 + D_DATA + D_WRITE + D_BIG	;TSS, Stack
mem_data_Mod_LDT_size equ $-mem_data_Mod_LDT

mem_data_Mod_TSS:
.tss:   dw   0    ;Task Link
	   dw   0    ;reserved

		dd	task_struc.StackSize-4	;esp0
		dw	mem_data_Mod_LDT.tss_stack - mem_data_Mod_LDT + 4
		dw   0    ;reserved
		dd   0    ;esp1
		dw   0    ;ss1
		dw   0    ;reserved
		dd   0    ;esp2
		dw   0    ;ss2
		dw   0    ;reserved

		dd   (mem_PD << 12) ;cr3/PDBR

		dd   200h	;eip = first loaded file(+1 sector)
		dd   202h    ;eflags

		dd   0    ;eax
		dd   0    ;ecx
		dd   0    ;edx
		dd   0    ;ebx

		dd   0FFCh ;esp
		dd   0    ;ebp

		dd   0    ;esi
		dd   0    ;edi

		dw   mem_data_Mod_LDT.data_sel - mem_data_Mod_LDT + 4    ;es
		dw   0    ;    Reserved
		dw   mem_data_Mod_LDT.code_sel - mem_data_Mod_LDT + 4 + 1 	;cs	+ TI + RPL1
		dw   0    ;    Reserved
		dw	mem_data_Mod_LDT.stack	 - mem_data_Mod_LDT + 4 + 1	;ss
		dw   0    ;    Reserved
		dw	mem_data_Mod_LDT.data_sel - mem_data_Mod_LDT + 4	;ds
		dw   0    ;    Reserved
		dw	mem_data_Mod_LDT.data_sel - mem_data_Mod_LDT + 4	;fs
		dw   0    ;    Reserved
		dw	mem_data_Mod_LDT.data_sel - mem_data_Mod_LDT + 4	;gs
		dw   0    ;    Reserved
		dw   Mod_LDT_sel    ;LDT
		dw   0    ;    Reserved

		dw   0    ;trap(bit0)
		dw   (.iobase - .tss)    ;IO map Base Address
.iobase:

mem_data_Mod_TSS_size equ $-mem_data_Mod_TSS