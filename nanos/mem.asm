;constants
global lin_kern
global pages
extern Kernel_Size

;The rest is in pages, Example: lin_PD*pages=linear address to Page Directory
pages	equ 1000h

;Descriptors
desc_null equ 0   ;Null
desc_call equ 1   ;Call Gate
desc_kern equ 2   ;Code(Kernel)
desc_data equ 3   ;Data(0-4G)

desc_Idle_TSS equ 4   ;Multitasking TSS
desc_Stack equ 5   ;Idle TSS Stack

desc_Mod_LDT	equ 6	;Loader module
desc_Mod_TSS	equ 7	;Loader module task

;Selectors
null_sel equ (desc_null << 3)
call_sel equ (desc_call << 3)
kern_sel equ (desc_kern << 3)
data_sel equ (desc_data << 3)

Idle_TSS_sel equ (desc_Idle_TSS << 3)
Stack_sel	equ (desc_Stack << 3)

Mod_LDT_sel	equ 6 << 3	;Loader module
Mod_TSS_sel	equ 7 << 3	;Loader module task

;Physical memory
mem_PD	equ	1

;Linear memory
lin_PT	equ 0
size_PT	equ 1024 ;PT(1)    All Page tables

lin_BIOS	equ	lin_PT + size_PT	
size_BIOS	equ	1
lin_IDT	equ lin_BIOS + size_BIOS
size_IDT	equ 1    ;IDT(Actually 0.5 pages)
lin_TL	equ lin_IDT + size_IDT
size_TL	equ 1    ;Task List(8192 tasks = 8 pages) 1024 tasks[4Byte] = 1p
lin_GDT	equ lin_TL+size_TL
size_GDT	equ 16   ;GDT(1)

lin_FLM	equ lin_GDT + size_GDT
size_FLM	equ 1

lin_Stack	equ lin_FLM + size_FLM		;Kernel Stack
size_Stack equ 1

;Module segments
;Stack
lin_Mod_Stack	equ	lin_Stack+size_Stack
size_Mod_Stack	equ	1
;LDT
lin_Mod_LDT	equ	lin_Mod_Stack + size_Mod_Stack
size_Mod_LDT	equ	1
;TSS
lin_Mod_TSS	equ	lin_Mod_LDT + size_Mod_LDT
size_Mod_TSS	equ	1
;Module end

lin_kern	equ lin_Mod_TSS + size_Mod_TSS
size_kern	equ Kernel_Size ;Kernel

lin_FPT	equ lin_kern + size_kern
;size_FPT equ System_Mem_FreePT
             ;Free Page Table(1 Page: 4MB physical)
             
lin_Mod_Data	equ	1024 * 2 + 512