[bits 32]

GLOBAL start
SEGMENT .text

RS232_Base	equ	3F8h
RS232_Line_Status	equ	5
RS232_Data	equ	0

start:
	
		;test if ready to send

		rdtsc
	again:		
		mov	dx, RS232_Base + RS232_Line_Status
		mov	cx, 0FFFFh
		.ready_to_send:
			in	al, dx
			bt	ax, 5
			jc	.ready
		loop	.ready_to_send
		
		.ready
		;send byte
		mov	al, ah
		mov	dx, RS232_Base + RS232_Data
		out	dx, al
	jmp	again
			
SEGMENT .data
	dd 45
	
SEGMENT .bss
	resd	5
		
