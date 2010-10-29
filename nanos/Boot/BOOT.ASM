%include '../const.asm'

%include 'macro.asm'

;===================================
;Nanos Boot Loader
;
; - Report to RS232 9600bps 8 bits 1 stop no flow control
;
; 1 Read fat table
; 2 Load:
;	nanos.bin:memseg
;	nanos.ini
;		files in nanos.ini
; 3 Start Pmode
; 4 jump to startup code
;===================================


;Start
[BITS 16]
[ORG 0000h]

%include 'fat12.asm'

start:
	cli
	push	byte 0
	pop	ss
	mov  sp, 07C00h
	sti
		
	jmp	07C0h:.start
.start:
	push	cs
	pop	ds

	;show bootloader message
	disp osstart, osend
	
FAT:
	call	reset	;reset disk
	
	;in reverse order
	Dat_Seg	equ	2000h
	FAT_Seg	equ	Dat_Seg - FATSz * BPB_BytsPerSec / 10h
	Dir_Seg	equ	FAT_Seg - RootDirSectors * BPB_BytsPerSec / 10h
	%if Dir_Seg < 7E0h
		%error "tables overwrites boot code"
	%endif
	
		
	;Load Directory:
	mov	ax, FirstRootDirSecNum
	mov	cx, RootDirSectors
	mov	dx, Dir_Seg
	call	Load_Sectors

	;Load FAT
	mov	ax, BPB_RsvdSecCnt
	mov	cx, FATSz
	;mov	dx, FAT_Seg
	call	Load_Sectors
	
	;Loader
		;Load nanos.bin
		mov	si, nanos_loader
		call	Load_File	;(ds:si filename, dx Segment)
		
		;Save nanos.dat memory segment
		push	dx
		
			;Load nanos.dat
			mov	si, init_file
			call	Load_File	;(ds:si filename, dx Segment)

		pop	bx

	;analyze nanos.dat
		mov	ds, bx
		xor	si, si	;ds:si = nanos.dat
	
	
	scan_dat:		
		cmp	[si], byte ':'
		jne	done
		
		add	si, byte 3		
		
		mov	ax, dx
		sub	ax, bx
		mov	[si + 11], ax		;Save start of file
		
		push	si
		push	bx
			call	Load_File	;(ds:si filename, dx Segment)
		pop	bx
		pop	si
		
		add	si, byte 11+2	;step over filename + cr + lf
		
		jmp	scan_dat
		
	done:
	;Save next segment after the last data segment
	xor	si, si	;ds:si = nanos.dat
	add	si, Loader_init.data_end
	mov	[si], dx		;dx = segment after loaded data
	
	mov	bx, ds

	cli
	push	cs
	pop	ds
		
	;Turn off floppy motor
	mov	dx, 03f2h
	mov	al, 0
	out	dx, al

	;Start PMode
	%include 'pmode.asm'	

reboot:
	mov	al, 'X'
	call	Cdisp
	jmp $
	
Disk:
	%include 'disk.asm'

Cdisp: ;Display Character(al)
	pusha
		mov  ah, 0Eh
		int  10h
	popa
	ret
	
printstring:
	mov  ah, 0Eh
	mov  bx, 0007h
nextcs:
	lodsb
	int  10h	;Print to screen
	loop nextcs
	ret

nanos_loader:	db 'NANOS   BIN'
init_file:	db 'NANOS   DAT'

osstart:	db 'Loading: '
osend:

;GDT
align 4	;even 4 byte

	nsel EQU 3
gdtr:	dw nsel*8-1	; GDT Limit
	dd 	(gdt-8-$$) + 7c00h			; GDT Base

	;Null selector
	
gdt:

		;Data selector
	dw 0FFFFh	;Limit
	dw 0		;Base0-15
	db 0		;Base16-23
	db 10010010b	;Present, DPL0 Writeable
	db 11001111b	;Granularity, Big, Limit
	db 0		;Base24-31

		;Code selector
	dw 0FFFFh	;Limit
	dw 0		;Base0-15
	db 0		;Base16-23
	db 10011010b	;Present, DPL0 Readable
	db 11001111b	;Granularity, Big, Limit
	db 0		;Base24-31

end:	 ;boot sign, so that the bios will boot on the drive
	times 200h-2-($-$$)	 db 0h
	db 55h, 0AAh