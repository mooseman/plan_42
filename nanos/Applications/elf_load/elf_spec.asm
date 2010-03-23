;
;	ELF Specification Data
;

ELF:
.Head		equ	0
	.Ident		equ	0
		.i_Magic	equ	0	;7F,'E','L','F'
		.i_Class	equ	4	
		.i_Data	equ	5
		.i_Ver	equ	6
		.i_Pad	equ	7
	.Type	equ	10h
	.Machine	equ	12h
	.Version	equ	14h
	.Entry	equ	18h
	.Phoff	equ	1Ch
	.Shoff	equ	20h
	.Flags	equ	24h
	.hsize	equ	28h
	.pesize	equ	2Ah
	.phnum	equ	2Ch
	.sesize	equ	2Eh
	.shnum	equ	30h
	.strindex	equ	32h	
	
	
;Program Header Entry:
	.p_Type		equ	 0h
	.p_Offset		equ	 4h
	.p_Vaddr		equ	 8h
	.p_Paddr		equ	0Ch
	.p_FileSize	equ	10h
	.p_MemSize	equ	14h
	.p_Flags		equ	18h
	.p_Align		equ	1Ch
	
	
	.p_Type_Load	equ	1
	
	.p_Flag_X		equ	1
	.p_Flag_W		equ	2
	.p_Flag_R		equ	4
	
	