Load_File:	;(ds:si filename, dx Segment)
	;ds:si = pointer to filename(FAT format)
	;dx = segment to load the file

	mov	al, '/'
	call Cdisp

	mov	ax, Dir_Seg
	mov	es, ax
	xor	di, di
	cld
	mov	cx, BPB_RootEntCnt
	
	.filename:
	push	cx
	push	si
		mov	cx, 11
	
		rep	cmpsb
		je	.found
	pop	si
	pop	cx
	;Search next
	and	di, 0FFE0h
	add	di, byte 32
	loop .filename
	
	;File not found
	jmp	reboot
	
		.found:	
	pop	si
	pop	cx	;fix stack
	
	mov	ax, [es:di - 11 + DIR_FstClus]
	;ax = cluster
	
	push	word FAT_Seg
	pop	es
	
	
	.check:
	;Check cluster pointer
	cmp	ax, 0FF8h		;EOF?
	jae	.done
	cmp	ax, 0		;cluster 0 - error
	je	.done
	
	;save cluster
	push	ax
	push	es
	
		;cluster to sector
		add	ax, byte FirstDataSector - 2

		;load segment
		;es = segment
		;ax = sector
		mov	es, dx
		call	Load_Sector
		add	dx, byte BPB_BytsPerSec / 10h
		
	pop	es		
	pop	ax
			
	;Get next cluster
	mov	bx, ax
	mov	si, ax
	shr	ax, 1
	add	si, ax	;si = ax + ax/2
	mov	ax, [es:si]
	shr	bx, 1	;Test bit 0
	jc	.odd
	shl	ax, 4
	.odd
	shr	ax, 4
	;ax = new cluster
	jmp	.check
		
	.done
ret
	


;Reset disk
reset:
	mov  cx, 10h   ;times to retry
	.loop:   ;Reset
	push cx

		xor  ah, ah	;00: reset disk
		mov	dl, BS_DrvNum	;dl=drive
		int  13h	   ;Reset disk
		jc	.fail
	pop	cx
	ret

		;reset failure
		.fail:
		mov  al, 'x'
		call Cdisp
	pop  cx
	loop .loop
	jmp  reboot

;INT 13 - DISK - RESET DISK SYSTEM
;	AH = 00h
;	DL = drive (if bit 7 is set both hard disks and floppy disks reset)
;Return: AH = status (see #0166)
;	CF clear if successful (returned AH=00h)
;	CF set on error


Load_Sectors:
	;ax = start sector
	;cx = number of sectors
	;dx = segment
	.dir:
	mov	es, dx
	call	Load_Sector ;ax = sector number, es = target
	inc	ax
	add	dx, byte BPB_BytsPerSec / 10h
	loop	.dir
	ret
	
	
;Load 1 Sector
;es = segment
;ax = sector
Load_Sector:
	push	ax
		mov	al, '.'
		call Cdisp
	pop	ax
	
	;Try 10 times
	push	cx
		mov	cx, 10
		.retry:
		pusha
			
			;Check max number
			cmp	ax, BPB_TotSec16
			jb	.chs
			jmp	reboot
			
			.chs:
			;Convert sector to CHS 
			xor	dx, dx
			mov	bx, BPB_SecPerTrk
			div	bx		;ax = ax / SectPerTrack
			mov	cl, dl						;dx = ax % SectPerTrack
			inc	cl		;cl = Sector number(1-63)
						;assume dl = 0
			shr	ax, 1	;since BPB_NumHeads = 2
			rcl	dh, 1	;dh = head(0-1)
			mov	ch, al	;ch = cylinder/track
			
			;Drivenumber and memory location
			mov	dl, BS_DrvNum

			xor	bx, bx	;Offset, (es = segment)
			mov	ax, 0201h;low byte: number of sectors to read
			
			int  13h
			jc	.error
		popa
	pop	cx
	ret

			.error:
			call	reset
		popa
		loop .retry
		jmp	reboot
	

;INT 13 - DISK - READ SECTOR(S) INTO MEMORY
;	AH = 02h
;	AL = number of sectors to read (must be nonzero)
;	CH = low eight bits of cylinder number
;	CL = sector number 1-63 (bits 0-5)
;		 high two bits of cylinder (bits 6-7, hard disk only)
;	DH = head number
;	DL = drive number (bit 7 set for hard disk)
;	ES:BX -> data buffer
;Return: CF set on error
;		if AH = 11h (corrected ECC error), AL = burst length
;	CF clear if successful
;	AH = status (see #0166)
;	AL = number of sectors transferred (only valid if CF set for some
;		  BIOSes)
;Notes:	errors on a floppy may be due to the motor failing to spin up quickly
;	  enough; the read should be retried at least three times, resetting
;	  the disk with AH=00h between attempts
