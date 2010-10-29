global   mem

;=========Placement of Loader + Data=============
memseg   equ  2000h ;start memory: segment
mem      equ  (memseg * 10h)

stack_esp   equ 1000h - 4

IRQBase	equ	20h

;Nanos Data Area
;Where Nanos places its parameters

Loader_init: ;Location of data
.data_end	equ	200h-4	;next segment after loaded data




Nanos_init:
;BIOS memory:
;0	old IDT
;0	Nanos Data
.MemSize		equ	0
.lin_FPT		equ	4
.lin_FPT_Size	equ	8

;400h	BIOS data

