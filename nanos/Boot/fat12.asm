;FAT12 data

BPB_BytsPerSec	equ	0200h
BPB_SecPerClus	equ	1
BPB_RsvdSecCnt	equ	1
BPB_NumFATs	equ	2
BPB_RootEntCnt	equ	00E0h
BPB_TotSec16	equ	0B40h
BPB_Media		equ	0F0h
BPB_FATSz16	equ	9
BPB_SecPerTrk	equ	0012h
BPB_NumHeads	equ	2
BPB_HiddSec	equ	0
BPB_TotSec32	equ	0
	
BS_DrvNum		equ	00h
BS_Reserved1	equ	00h
BS_BootSig	equ	29h
BS_VolID		equ	1086ADCBh

jmp short start
times 3-($-$$) nop
	db 'MSDOS5.0'	;OEM Name
	dw BPB_BytsPerSec
	db BPB_SecPerClus
	dw BPB_RsvdSecCnt
	db BPB_NumFATs
	dw BPB_RootEntCnt
	dw BPB_TotSec16
	db BPB_Media
	dw BPB_FATSz16
	dw BPB_SecPerTrk
	dw BPB_NumHeads
	dd BPB_HiddSec
	dd BPB_TotSec32
	
	db BS_DrvNum
	db BS_Reserved1
	db BS_BootSig
	dd BS_VolID
	db 'NO NAME    '	; 11 bytes Volume Label
	db 'FAT12   '		; 8 bytes  File System
times 62 - ($-$$)	db 'E';rror
times ($-$$) - 62	db 'E';rror

;Boot sector:
FAT_Offset:
;FAT:
;	BS_jmpBoot		equ	0  ;3
;	BS_OEMName		equ	3  ;8
;	.BPB_BytsPerSec	equ	11 ;2
;	.BPB_SecPerClus	equ	13 ;1
;	BPB_RsvdSecCnt	equ	14 ;2
;	BPB_NumFATs		equ	16 ;1
;	BPB_RootEntCnt	equ	17 ;2
;	BPB_TotSec16		equ	19 ;2
;	BPB_Media		equ	21 ;1
;	BPB_FATSz16		equ	22 ;2
;	BPB_SecPerTrk		equ	24 ;2
;	BPB_NumHeads		equ	26 ;2
;	BPB_HiddSec		equ	28 ;4
;	BPB_TotSec32		equ	32 ;4
	
;FAT16:	;FAT12, FAT16
;	.BS_DrvNum	equ	36 ;1
;	.BS_Reserved1	equ	37 ;1
;	.BS_BootSig	equ	38 ;1
;	.BS_VolID		equ	39 ;4 
;	.BS_VolLab	equ	43 ;11
;	.BS_FilSysType	equ	54 ;8

	
;FAT Byte Directory Entry Structure
	DIR_Name			equ	0	;11
	DIR_Attr			equ	11	;1
	DIR_NTRes			equ	12	;1
	DIR_CrtTimeTenth	equ	13	;1
	DIR_CrtTime		equ	14	;2
	DIR_CrtDate		equ	16	;2
	DIR_LstAccDate		equ	18	;2	
	DIR_FstClusHI		equ	20	;2	First high cluster number(FAT32 only)
	DIR_WrtTime		equ	22	;2
	DIR_WrtDate		equ	24	;2
	
	DIR_FstClus		equ	26	;2	First low cluster number
	DIR_FstClusLO		equ	26	;2	First low cluster number
	
	DIR_FileSize		equ	28	;4
	
;Attributes
	ATTR_READ_ONLY	equ 0x01
	ATTR_HIDDEN	equ 0x02
	ATTR_SYSTEM	equ 0x04
	ATTR_VOLUME_ID	equ 0x08
	ATTR_DIRECTORY equ 0x10
	ATTR_ARCHIVE	equ 0x20
	ATTR_LONG_NAME equ ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID

;Macron

FATSz equ BPB_FATSz16

FirstRootDirSecNum equ BPB_RsvdSecCnt + (BPB_NumFATs * FATSz);
RootDirSectors equ ((BPB_RootEntCnt * 32) + (BPB_BytsPerSec - 1)) / BPB_BytsPerSec;

TotSec equ BPB_TotSec16

;N = cluster number

FirstDataSector equ BPB_RsvdSecCnt + (BPB_NumFATs * FATSz) + RootDirSectors

;%Macro
;FirstSectorofCluster = ((N - 2) * BPB_SecPerClus) + FirstDataSector

;FATOffset = N + (N / 2)

;ThisFATSecNum = BPB_ResvdSecCnt + (FATOffset / BPB_BytsPerSec);
;ThisFATEntOffset = REM(FATOffset / BPB_BytsPerSec);

;if (FATContents >= 0FF8h) EOF=True

