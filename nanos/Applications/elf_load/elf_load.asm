;=========================
;
;	ELF Loader
;
; Create modules based upon ELF data
;=========================
;
;Offset:	Contents
;0		nanos.bin
;200h	First loaded file(this)
;
;Nanos.bin:
;LoadEntry:
;Byte	Size	Contents
;0		1	':'
;1		11	FAT Filename("FAT format"): 'FILE	ELF'
;12		2	File location in memory(in segments)
;===================================================	
;

%include 'elf_spec.asm'
%include 'nanos.inc'
[bits 32]

[org 200h]


dat:
	.sign	equ	0	;=':'
	.DPL		equ	1	;DPL '0'-'3'
	.res		equ	2	;Reserved/Unknown
	.Name	equ	3	;"FILE	ELF"
	.Seg		equ	14	;Segment Offset
	.EntSize	equ	16
	.EntSize2	equ	4
	
start:
	xor	edx, edx
	add	edx, byte dat.EntSize	;edx = first file after this
	
Load:
	;loop through all ELF modules

	cmp	[edx+dat.sign], byte ':'	;Start of Filename
	jne near connect
	
	;File found, load it
	mov	[FilePtr], edx	;Save Ptr
	
	movzx ebx, word [edx+dat.Seg]
	shl	ebx, 4
	;ebx = location of file in memory
	mov	[FileMem], ebx

;Check if an ELF file
	
	;Header
	mov	esi, CompareData
	mov	edi, ebx
	mov	ecx, Compare_Size
	rep	cmpsb
	jne near LoadNext

;Create a module
	mov	ecx, 100h
	mov	eax, module.create		;(eax Selector) == (ecx LDT Size)
	call	Kernel
	jc near LoadNext
	mov	[ModSel], eax		;Save Module segment
	
;Create a Task
	;Create Task
	mov	eax, mult.create		;(eax = Selector) == ()
	call	Kernel
	jc	$
	mov	[TaskSel], eax
	
	;Get Task Data
	mov	edx, eax
	mov	ebx, TSS
	mov	eax, mult.get		;() == (edx TSS Selector, ds:ebx TSS segment data)
	call	Kernel
	
	;get Privilege level
	mov	edx, [FilePtr]	;edx = file data
	mov	dl, [edx + dat.DPL]
	sub	dl, '0'
	and	edx, 011b		;edx = DPL
	
	;SS
	StackSize	equ	1000h	
	or	edx, 00020004h	;Data(W), Any(LDT) + DPL
	mov	ecx, StackSize
	mov	eax, desc.create_data		;(eax = Selector) == (edx [Settings][Selector], ecx Size)
	call	Kernel
	jc	$

	;Move SS to module
	mov	edx, eax	;source
	mov	ebx, [ModSel]
	shl	ebx, 10h
	arpl	bx, dx		
	add	ebx, byte 0100b	;ebx = [module][Any selector in module, DPL same as SS]
	mov	eax, module.move		;(eax Selector) == (edx Source selector, ebx Target [module][selector])
	call	Kernel
	jc	$
	mov	[TSS.ss], ax
	
	
;Load all Segments in the Program Header Table
	mov	ebx, [FileMem]
	mov	esi, [ebx + ELF.Phoff]
	add	esi, ebx
	movzx ecx, word [ebx + ELF.phnum]	;ecx = entry count
	
	
	;esi = start of entry
	;ecx = entries left

	LoadSegment:
	cmp	ecx, 0
	je near SegmentsDone
	dec	ecx
	
	;Check Segment type:
	cmp	[esi + ELF.p_Type], dword ELF.p_Type_Load
	jne near NextSegment

	push	ecx
		;Get size needed
		mov	ecx, [esi + ELF.p_Vaddr]
		add	ecx, [esi + ELF.p_MemSize]	;ecx = Vaddress + Memsize

		;Get DPL
		mov	edx, [FilePtr]	;edx = file data
		mov	dl, [edx + dat.DPL]
		sub	dl, '0'
		and	edx, 011b		;edx = DPL

		;Create Descriptor			
		or	edx, 00020004h	;Data(W), Any(LDT)
		mov	eax, desc.create_data		;(eax = Selector) == (edx [Settings][Selector], ecx Size)
		call	Kernel
		jc near SegmentDone
		mov	[DataSel], eax
		
		;Fill With Pages		
		mov	edx, 00060000h	;User, Write
		mov	dx, [DataSel]
		mov	ebx, [esi + ELF.p_Vaddr]
		mov	ecx, [esi + ELF.p_MemSize]
		mov	eax, page.alloc		;() == (edx = [Settings][Selector], ebx = Base, ecx = Size)
		call	Kernel
		jc	$
		
		;Copy data
		push	esi
			mov	ecx, [esi + ELF.p_FileSize]
			push dword [DataSel]
			pop	es
			mov	edi, [esi + ELF.p_Vaddr]
			
			mov	esi, [esi + ELF.p_Offset]
			add	esi, [FileMem]
			rep	movsb
			
		pop	esi
		;Zero rest of data
		mov	ecx, [esi + ELF.p_MemSize]
		sub	ecx, [esi + ELF.p_FileSize]
		xor	eax, eax
		rep	stosb
		push	ds
		pop	es	;Restore es
		
						
		;Get Segment Settings
		mov	edx, [esi + ELF.p_Flags]		;RWX Flags
		bt	edx, 0
		jc	LoadCode
		LoadData:
			and	edx, 0010b	;Writeable
			jmp	LoadCodeData
		LoadCode:	
			shr	edx, 1
			and	edx, 0010b	;Readable
			add	edx, byte 8	;Code bit
		LoadCodeData:
		shl	edx, 10h
		mov	dx, [DataSel]
		
		;Change Descriptor
		mov	eax, desc.set		;() == (edx [Settings][Selector])
		call	Kernel
		jc	$
		
		;Move descriptor to module
		mov	edx, [DataSel]
		mov	ebx, [ModSel]
		shl	ebx, 10h
		mov	bx, 4
		arpl	bx, dx
		mov	eax, module.move		;(eax Selector) == (edx Source selector, ebx Target [module][selector])
		call	Kernel
		
		
		;Save Selector for TSS
		mov	edx, [esi + ELF.p_Flags]		;RWX Flags
		bt	edx, 0
		jc	WriteCode
		WriteData:
			cmp word [TSS.ds], 0
			jne	WriteDone		;Already taken
			mov	[TSS.ds], ax
		jmp	WriteDone
		WriteCode:
			cmp word [TSS.cs], 0
			jne	WriteDone		;Already taken
			mov	[TSS.cs], ax		
		WriteDone:
			
		SegmentDone:
	pop	ecx
	
	NextSegment:
	mov	ebx, [FileMem]
	movzx ebx, word [ebx + ELF.pesize]
	add	esi, ebx
	jmp	LoadSegment
	
	SegmentsDone:
	;All segments created adn moved
	
	;Set TSS Data
	mov	edx, [TaskSel]
	mov	ebx, TSS
	mov	eax, mult.set		;() == (edx TSS Selector, ds:ebx TSS segment data)
	call	Kernel
	jc	$
	
	;Move Task
	mov	edx, [TaskSel]
	mov	ebx, [ModSel]
	mov	eax, module.movetask	;() == (edx Source Task selector, ebx Target Module)
	call	Kernel
	jc	$
	
	;Start task
	mov	eax, mult.add		;() == (dx = TSS Selector)
	call	Kernel
	jc	$

	LoadNext:	;Module
	
	;Move pointer to next file
	mov	edx, [FilePtr]
	add	edx, byte dat.EntSize
	jmp	Load



connect:
	;make connections between all modules
	;list = list of module selectors

	;Make all connections
	
	
	
	;Terminate this module
	jmp	connect
	
	
CompareData:
.Head:
	.Magic:	db	7Fh, 'E', 'L', 'F'
	.Class:	db	1	;32 Bit file
	.Encode:	db	1	;LSB Encoding
	.Version	db	1	;ELF Version
	.Pad:	times 16 - ($-.Head)	db 0
	
	.Type:	dw	2	;Executable
	.Machine:	dw	3	;Intel 386
			dd	1	;ELF Version
Compare_Size	equ $-.Head
	
	
;Module Data
FilePtr:		dd	0	;Pointer at file list

FileMem:		dd	0	;File offset

ModSel		dd	0	;Module Selector
TaskSel		dd	0	;Task Selector
DataSel		dd	0	;Data Selector

TSS:
.link dw	0	;Task Link
	dw	0	;reserved

	dd	0	;esp0
	dw	0	;ss0
	dw	0	;reserved
	dd	0	;esp1
	dw	0	;ss1
	dw	0	;reserved
	dd	0	;esp2
	dw	0	;ss2
	dw	0	;reserved

	dd	0	;cr3/PDBR

.eip	dd	0	;eip = start
	dd	202h	;eflags

	dd	0	;eax
	dd	0	;ecx
	dd	0	;edx
	dd	0	;ebx

	dd	0FFCh ;esp
	dd	0	;ebp

	dd	0	;esi
	dd	0	;edi

	dw	0	;es
	dw	0	;	Reserved
.cs	dw	0	;cs
	dw	0	;	Reserved
.ss	dw	0	;ss
	dw	0	;	Reserved
.ds	dw	0	;ds
	dw	0	;	Reserved
	dw	0	;fs
	dw	0	;	Reserved
	dw	0	;gs
	dw	0	;	Reserved
	dw	0	;ldt
	dw	0	;	Reserved

	dw	0	;trap(bit0)
	dw	(.iobase - TSS)	;IO map Base Address
.iobase:

;error checking
times	68h-(.iobase-TSS)  nop
times	(.iobase-TSS)-68h  nop
	
	