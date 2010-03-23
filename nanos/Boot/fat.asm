;FAT12/26/32 data


;Boot sector:
FAT:
	BS_jmpBoot		equ	0  ;3
	BS_OEMName		equ	3  ;8
	BPB_BytsPerSec	equ	11 ;2
	BPB_SecPerClus	equ	13 ;1
	BPB_RsvdSecCnt	equ	14 ;2
	BPB_NumFATs		equ	16 ;1
	BPB_RootEntCnt	equ	17 ;2
	BPB_TotSec16		equ	19 ;2
	BPB_Media		equ	21 ;1
	BPB_FATSz16		equ	22 ;2
	BPB_SecPerTrk		equ	24 ;2
	BPB_NumHeads		equ	26 ;2
	BPB_HiddSec		equ	28 ;4
	BPB_TotSec32		equ	32 ;4
	
FAT16:	;FAT12, FAT16
	.BS_DrvNum	equ	36 ;1
	.BS_Reserved1	equ	37 ;1
	.BS_BootSig	equ	38 ;1
	.BS_VolID		equ	39 ;4 
	.BS_VolLab	equ	43 ;11
	.BS_FilSysType	equ	54 ;8

FAT32:	;FAT32
	.BPB_FATSz32	equ	36 ;4
	.BPB_ExtFlags	equ	40 ;2
	.BPB_FSVer	equ	42 ;2
	.BPB_RootClus	equ	44 ;4	---
	.BPB_FSInfo	equ	48 ;2
	.BPB_BkBootSec	equ	50 ;2
	.BPB_Reserved	equ	52 ;12
	.BS_DrvNum	equ	64 ;1
	.BS_Reserved1	equ	65 ;1
	.BS_BootSig	equ	66 ;1
	.BS_VolID		equ	67 ;4
	.BS_VolLab	equ	71 ;11
	.BS_FilSysType	equ	82 ;8

	
;FSInfo sector
FSInfo:
	FSI_LeadSig	equ	0   ;4	0x41615252
	FSI_Reserved1	equ	4   ;480
	FSI_StrucSig	equ	484 ;4	0x61417272
	FSI_Free_Count	equ	488 ;4
	FSI_Nxt_Free	equ	492 ;4
	FSI_Reserved2	equ	496 ;12
	FSI_TrailSig	equ	508 ;4	0xAA550000

;FAT 32 Byte Directory Entry Structure
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

;Only FAT12 FAT16
;RootDirSectors = ((BPB_RootEntCnt * 32) + (BPB_BytsPerSec - 1)) / BPB_BytsPerSec;


%macro FATSz 0
;If(BPB_FATSz16 != 0)
;	FATSz = BPB_FATSz16;
;Else
;	FATSz = BPB_FATSz32;
;EndIf
%endmacro

%macro TotSec 0
;If(BPB_TotSec16 != 0)
;	TotSec = BPB_TotSec16;
;Else
;	TotSec = BPB_TotSec32;
%endmacro

;FirstDataSector = BPB_ResvdSecCnt + (BPB_NumFATs * FATSz) + RootDirSectors

;FirstSectorofCluster = ((N - 2) * FAT.BPB_SecPerClus) + FirstDataSector

;FAT Type:
;DataSec = TotSec - (BPB_ResvdSecCnt + (BPB_NumFATs * FATSz) + RootDirSectors);
;CountofClusters = DataSec / BPB_SecPerClus;

;If(CountofClusters < 4085)	Volume is FAT12
;If(CountofClusters < 65525)	Volume is FAT16
;else					Volume is FAT32

;----
;If(FATType == FAT16)
;	FATOffset = N * 2;
;Else if (FATType == FAT32)
;	FATOffset = N * 4;
;endif

;Where in FAT table a spceific cluster is specified
;Linked list structure
;ThisFATSecNum = BPB_ResvdSecCnt + (FATOffset / BPB_BytsPerSec);
;ThisFATEntOffset = REM(FATOffset / BPB_BytsPerSec);


