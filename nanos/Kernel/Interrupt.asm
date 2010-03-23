;=====================
;Interrupt handler
;
;	 0-1F	Exceptions
;	20-2F	IRQ 0-F
;	30-FF	Interrupt 30-FF


;==================================
; Exceptions
;
%macro exception 1
global   Interrupt.exception%1
.exception%1:
	mov	eax, %1h
	jmp	.exception
%endmacro

.exception:
	;Fix ds
	mov	dx, cs
	mov	ds, dx
	
	Print	.t_exc

	PrintHex	eax, 2	
	
	Print	.t_err

	pop	eax
	PrintHex eax, 8

	Print	.t_eip

	pop	eax
	PrintHex eax, 8
	
	Print	.t_cs

	pop	eax
	PrintHex eax, 4
	
	Print	.t_end

	sti
	
	;Fix ds
	mov	ax, data_sel
	mov	ds, ax
	
	cli
	.everlasting:
		
		;Stop current task
	;	str	ax
	;	call	mult.remove	;() == (ax = TSS Selector)

	hlt
	jmp	.everlasting

.t_exc:
	db	10, 10, 10, 'Exception error: ', 0
.t_err:
	db	'          ', 10, '     Error code: ', 0
.t_eip:
	db	'    ', 10, '            EIP: ', 0
.t_cs:
	db	'    ', 10, '             CS: ', 0
.t_end:
	db	'        ', 10, 10, 'Interrupts disabled, System halted', 0
	
	
	
    exception 00
    exception 01
    exception 02
    exception 03
    exception 04
    exception 05
    exception 06
    exception 07
    exception 08
    exception 09
    exception 0A
    exception 0B
    exception 0C
    exception 0D
    exception 0E
    exception 0F
    exception 10
    exception 11
    exception 12
    exception 13
    exception 14
    exception 15
    exception 16
    exception 17
    exception 18
    exception 19
    exception 1A
    exception 1B
    exception 1C
    exception 1D
    exception 1E
    exception 1F

;==================================
; IRQs
; 20-2F
;
%macro IRQ 1
global   Interrupt.IRQ%1
.IRQ%1:
	pusha
	mov	eax, 0%1h
	jmp	.IRQ_handle
%endmacro

.IRQ_handle:
	PrintByte '£'
	;Look up in table
	;Start task
	;send EOI to PIC
	popa
	iret

IRQ 0
IRQ 1
IRQ 2
IRQ 3
IRQ 4
IRQ 5
IRQ 6
IRQ 7
IRQ 8
IRQ 9
IRQ A
IRQ B
IRQ C
IRQ D
IRQ E
IRQ F



;==================================
; Interrupts
; 30-FF
;
global	Interrupt.Interrupts
.Interrupts:
	pusha
	
	PrintByte '$'
	;Look up in table
	;Start task
	popa
	iret

