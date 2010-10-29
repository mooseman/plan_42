;
; Nanos Structures
;

;==============================================================================
; BIOS data, Nanos data
System:
	.Base	equ	lin_BIOS * pages
	
	.old_Interrupt	equ	System.Base +   0
	.BIOS_Data	equ	System.Base	+ 400h
	.Nanos_Data	equ	System.Base	+ 800h
	.Memsize		equ	System.Base	+ Nanos_init.MemSize
	.lin_FPT		equ	System.Base	+ Nanos_init.lin_FPT
	.lin_FPT_Size	equ	System.Base	+ Nanos_init.lin_FPT_Size
	

;==============================================================================
;Multitasking
multitasking_struc:
	;Task List
		.Head		equ	lin_TL * pages
		.HeadSize		equ	10h
		.Base		equ	.Head + .HeadSize
		.Size		equ	size_TL * pages - .HeadSize
		.Entry_Size_2	equ  2
		.Entry_Size	equ	4 ;bytes

	;Header:
		.current	equ	0	;4 byte Pointer at current running entry
		.last	equ	4	;4 byte Pointer at last entry
		.float	equ	8	;2 byte TSS selector of current floating point state owner

	;Entry structure:
		;.current	equ	0	;2 byte Running TSS	(= interface if not equal Original TSS)
		.original	equ	2	;2 byte Original TSS


;==============================================================================
;Module
;	one LDT
module_struc:	;Module Data(= LDT base & FFFFF000h)
	.Name		equ	 0h	;10h bytes
	;.free		equ	14h	;Next entry
	
	.Interface		equ	80h	;Interface List
		.InterfaceEntSize	equ	 8h	;Size of structure
		.InterfaceEntSize2	equ	 3	;Size of structure 2Log
		.InterfaceCount	equ	10h	;number of interfaces
		.InterfaceSize		equ	.InterfaceCount * .InterfaceEntSize
	
	.LDT			equ	.Interface + .InterfaceSize

;Interface List
; inside Module
interface_struc:	;Interface entry structure
	.Type	equ	0
		.Type_present		equ	 1
		.Type_in_interface	equ	 2	;otherwise an out interface
		.Type_in_reg		equ	 4
		.Type_in_float		equ	 8
		.Type_in_copy		equ	10h	;Only one of these three
		.Type_in_page		equ	20h	; |
		.Type_in_desc		equ	30h	; /
		.Type_out_reg		equ	40h
		.Type_out_float	equ	80h
		.Type_Type_map		equ	0FFFFFF00h
	
	;In:  Connected: Module connected
	.Module	equ	4
	;Out: Caller(Process), 0 otherwise
	.Caller	equ	4
		
	.TSS		equ	6	;TSS to call/be called
	.end		equ	8	;end/Size of entry
	
%if module_struc.InterfaceEntSize != interface_struc.end
	%error 'Interface size wrong'
%endif
	
	
;==============================================================================
;Process/Interface (TSS)
task_struc:
	.TSS		equ	0	;TSS Data
	.Process	equ	100h	;Process/Interface Data	
	.Float	equ	200h	;Float Data
	.Stack	equ	400h	;Ring 0 Stack Data(descriptor in LDT)	
	.StackSize equ pages - .Stack
	;1000h	(next page)
	
;Process/Interface Data(100h bytes)
	.Name	equ	.Process + 0	;Name
	.Runtime	equ	.Process + 10h	;Runtime
	.Forward	equ	.Process + 18h	;Forward link(interface called)

;==============================================================================
;TSS/Data Descriptor:
;Available bit:
	;0 process(ordinary)
	;1 interface Task/Data - only removed by interface kernel code
		

;==============================================================================
;Paging Structures

Paging:
%define	Paging_Base	equ	[System.lin_FPT]		;dword with pointer to start of FPT
%define	Paging_Size	equ	[System.lin_FPT_Size]

	