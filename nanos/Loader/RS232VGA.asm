;
;RS232 Interface macros
;To be used for debugging
;
;Written by Peter Hultqvist, peter.hultqvist@neocoder.net
;

;Port Base Addresses*
;Port1	3F8h
;Port2	2F8h
;Port3	3E8h
;Port4	2E8h
;
;	*Base addresses may be different on different computers
RS232:

RS232_Base	equ	3F8h

;DLAB=0
RS232_Data	equ	0
RS232_Interrupt_Enable	equ	1
RS232_Interrupt_ID	equ	2
RS232_FIFO_Control	equ	2

;DLAB=1
RS232_DivisorLSB	equ	0
RS232_DivisorMSB	equ	1
RS232_Alternate_Function	equ	2

RS232_Line_Control	equ	3
RS232_Modem_Control	equ	4
RS232_Line_Status	equ	5
RS232_Modem_Status	equ	6
RS232_Scratch_Pad	equ	7

;Init code
pusha
	;Initiate RS232
		;Set DLAB = 0
		mov	dx, RS232_Base + RS232_Line_Control
		mov	al, 0
		out	dx, al

		;Disable Interrupts
		mov	dx, RS232_Base + RS232_Interrupt_Enable
		mov	al, 0
		out	dx, al

		;Set DLAB = 1
		mov	dx, RS232_Base + RS232_Line_Control
		mov	al, 80h
		out	dx, al
		
		;Set Baud Rate
		mov	dx, RS232_Base + RS232_DivisorLSB
		mov	al, 0Ch	;9600 kbps
		out	dx, al
		inc	dx
		mov	al, 0
		out	dx, al

		;Set DLAB = 0, + Line Control
		mov	dx, RS232_Base + RS232_Line_Control
		mov	al, 3	;8 bits, no parity, 1 stop
		out	dx, al
		
		;Disable FIFO
		mov	dx, RS232_Base + RS232_FIFO_Control
		mov	al, 0
		out	dx, al
	
	;Initiate VGA
		mov	edi, 0B8000h
		mov	esi, 0B8000h + 2*80
		mov	ecx, 2*80*24 / 4
		rep	movsd	;scroll one row
		
		mov	eax, 0E200E20h ;color' 'color' '
		mov	ecx, 2*80*2 / 4
		rep	stosd	;clear bottom row + one more row
	popa

	jmp	.end

%macro Print 1
	pusha
	mov	edx, %1
	call RS232.Print
	popa
%endmacro

.Print:
	.print_loop:
	mov	al, [edx]
	cmp	al, 0
	je	.print_done
	call	.PrintByte
	inc	edx
	jmp	.print_loop
	
	.print_done
	ret

	
.cur:	dd 0

%macro PrintByte 1
	pusha
	mov	al, %1
	call RS232.PrintByte
	popa
%endmacro

.PrintByte:
	pusha
	;al = byte to send/print
	;RS232
		mov	ah, al
		;test if ready to send
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
		
	;VGA	
		cmp	al, 10	;new line
		jne	.vga_print
			call	.scroll
			jmp	.over
		.vga_print:
		
		mov	ebx, 0B8000h + 2*80*24
		add	ebx, [RS232.cur]

		mov	[ebx], al
		add	ebx, 2
		add dword [RS232.cur], 2
		cmp	ebx, 0B8000h + 2*80*25
		jb	.over
		call	.scroll
		.over:
		call	.set_cursor
	popa
	ret

	.scroll:
		mov	edi, 0B8000h
		mov	esi, 0B8000h + 2*80
		mov	ecx, 2*80*25 / 4
		rep	movsd	;scroll one row
		mov	dword [RS232.cur], 0
		
		call	.set_cursor
	ret

%macro PrintHex 2
	pusha
	mov	eax, %1	
	mov	ecx, %2
	call RS232.PrintHex
	popa
%endmacro

.PrintHex:
	;Print a number in hex
	;in:	eax = number to be written
	;	ecx = number of nibbles(hex number)(1 to 8)
	
	mov	edx, eax
	
	shl	ecx, 2
	ror	edx, cl
	shr	ecx, 2
	
	
		
	.hex_loop:
		rol	edx, 4
	
		mov	al, dl
		and	al, 0Fh

		cmp	al, 10
		setb	ah
		dec	ah
		and	ah, 'A'-'0' -10		
		add	ah, '0'
		add	al, ah
		call	.PrintByte
		
		loop	.hex_loop
	ret
	
	
%macro PrintBin 2
	pusha
	mov	eax, %1	
	mov	ecx, %2
	call RS232.PrintBin
	popa
%endmacro
	
.PrintBin:
	;Print a number in binary
	;in:	eax = number to be written
	;	ecx = number of bits(1 to 32)
	mov	edx, eax
	ror	edx, cl
	
	.bin_loop:
		rcl	edx, 1	
		
		setc	al
		add	al, '0'
		call	.PrintByte
		
		loop	.bin_loop
	
	ret

.set_cursor:
	;moves the cursor to Screen.cursor
	mov	ebx, [RS232.cur]
	shr	ebx, 1
	add	ebx, 80*24
	
	mov	dx, 	03D4h	;Address
	mov	al,  0Fh		;low register
	out	dx,	al
	inc	dx
	mov	al,	bl
	out	dx,	al
	
	dec	dx
	mov	al,	0Eh		;high register
	out	dx,	al
	inc	dx
	mov	al,	bh
	out	dx,	al

	ret
.end:


%macro getPos 1
	mov	%1, dword [RS232.cur]
%endmacro

%macro setPos 1
	mov	dword [RS232.cur], %1
%endmacro
