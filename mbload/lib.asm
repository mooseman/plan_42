;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; MBLOAD-C: a Multiboot-compatible kernel loader
; that runs from the DOS command prompt.
;
; Chris Giese	<geezer@execpc.com>	http://my.execpc.com/~geezer
; Release date: Aug, 2003
; This code is public domain (no copyright).
; You can do whatever you want with it.
;
; EXPORTS:
; extern char g_cpu32, g_v86, g_dos, g_xms;
; extern unsigned long g_convmem_adr, g_convmem_size;
; extern unsigned long g_extmem_adr, g_extmem_size;
; extern unsigned long g_entry;
; extern unsigned g_num_ranges;
; extern range_t g_ranges[];
;
; void asm_init(void);
; void xms_exit(void);
; int copy_high(unsigned long dst_linear, char *src, unsigned count);
; void enter_pmode(void);
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%macro	IMP 1
	EXTERN _%1
	%define %1 _%1
%endmacro

%macro	EXP	1
	GLOBAL $_%1
	$_%1:
	$%1:
%endmacro

; IMPORTS
IMP _psp	; "_psp" is from Turbo C startup code
IMP g_mboot
IMP g_phys
IMP g_linear

; *** NOTE: must be same value as MAX_RANGES in C code
MAX_RANGES	EQU 32

; 24-byte memory range
; *** NOTE: must be same layout as range_t struct in C code
	struc range_t
.res:		resd 1	; size of range_t? GRUB docs aren't too clear...
.adr:		resd 1
.res_adr:	resd 1	; unused here; used for 64-bit range address
.size:		resd 1
.res_size:	resd 1	; unused here; used for 64-bit range size
.type:		resw 1
.res_type:	resw 1	; unused here; used for 32-bit range type
.len:		resb 0
	endstruc

SEGMENT _TEXT PUBLIC CLASS=CODE
SEGMENT _DATA PUBLIC CLASS=DATA
SEGMENT _BSS PUBLIC CLASS=BSS
SEGMENT _BSSEND PUBLIC CLASS=BSSEND

%ifdef TINY
GROUP DGROUP _TEXT _DATA _BSS _BSSEND
%else
GROUP DGROUP _DATA _BSS _BSSEND
%endif

SEGMENT _TEXT

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:		asm_init
; action:	checks for DOS, 32-bit CPU, XMS, etc.
;		gets BIOS map of free conventionl/extended memory
; in:		(nothing)
; out:		various global variables are set
; modifies:	(nothing)
; minimum CPU:	8088
; notes:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

EXP asm_init
	push es
	push dx
	push bx
	push ax
		mov [_real_ds],ds

; check for 32-bit CPU
		pushf
			pushf
			pop bx		; old FLAGS -> BX
			mov ax,bx
			xor ah,70h	; try changing b14 (NT)...
			push ax		; ... or b13:b12 (IOPL)
			popf
			pushf
			pop ax		; new FLAGS -> AX
		popf
		xor ah,bh		; 32-bit CPU if we changed NT...
		and ah,70h		; ...or IOPL
		mov [g_cpu32],ah

; test for DOS PSP to see if we booted from DOS or from a bootloader
; MBL and MBLOAD require DOS, but BING does not
		push ds
			mov ds,[_psp]
			cmp word [0],20CDh
		pop ds
		je got_dos
		jmp no_dos
got_dos:
		inc byte [g_dos]

; if DOS, check for XMS (HIMEM.SYS loaded)
		mov ax,4300h
		int 2Fh
		cmp al,80h
		jne no_xms
		inc byte [g_xms]

; if DOS and XMS, get XMS driver address
		mov ax,4310h
		int 2Fh
		mov [_xms_entry + 2],es
		mov [_xms_entry + 0],bx

; ...and get extended memory block from XMS
		xor al,al
		or al,[g_xms]
		je no_xms
		call xms_init
no_xms:
; allocate conventional memory via DOS
; 1. DOS gives .COM file all free conventional memory; so use
;    INT 21h AH=4Ah to reduce allocated memory to 64K
;    (Turbo C startup code already does this.)
%ifdef __WATCOMC__
		mov es,[_psp]
		mov bx,1000h
		mov ah,4Ah
		int 21h
		jc loheap_err
%endif

; 2. free the environment memory
		mov es,[_psp]
		push es
			mov es,[es:2Ch]
			mov ah,49h
			int 21h
		pop es

; 3. zero the environment pointer
		mov [es:2Ch],word 0

; 4. get size of DOS conventional memory to BX
		mov ah,48h
		mov bx,0FFFFh
		int 21h

; 5. allocate conventional memory
		mov ah,48h
		int 21h
		jc loheap_done

; 6. convert address and size from 16-byte paragraphs to bytes and store
		mov dx,16
		mul dx
		mov [g_convmem_adr + 0],ax
		mov [g_convmem_adr + 2],dx
		mov ax,bx
		mov dx,16
		mul dx
		mov [g_convmem_size + 0],ax
		mov [g_convmem_size + 2],dx
loheap_err:
loheap_done:
; if DOS and 32-bit CPU, check if CPU in Virtual 8086 mode
		xor ax,ax
		or al,[g_cpu32]
		je no_dos

		smsw ax			; 'SMSW' is a '286+ instruction
		and al,1
		mov [g_v86],al
no_dos:
; get map of conventional and extended memory ranges
		call get_memory_map
	pop ax
	pop bx
	pop dx
	pop es
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; MEMORY SIZE/LAYOUT DETECTION
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:		store_range
; action:	stores memory range (type, size, adr) in global list
; in:		CX:BX=linear base address of range,
;		DX:AX=size of range, DI=type of range,
;		[g_num_ranges] set
; out (too many ranges):CY=1
; out (success):CY=0
; modifies:	[g_num_ranges], one entry at [g_ranges]
; minimum CPU:	8088
; notes:	### - ranges are not guaranteed to be
;		- contiguous (OK),
;		- sorted in ascending order (uh-oh), or
;		- non-overlapping (uh-oh)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

store_range:
	push si

; too many ranges?
		cmp byte [g_num_ranges],MAX_RANGES
		jae sr_3

; ignore range if size==0
		push ax
			or ax,dx
		pop ax
		je sr_2

; form pointer to g_ranges[g_num_ranges]
		push ax
			mov al,range_t.len
			mul byte [g_num_ranges]
			add ax,g_ranges
			mov si,ax
		pop ax

; if this is not the first range...
		cmp si,g_ranges
		je sr_1

; ...check if type of this range is the same as previous range
		cmp di,[si - range_t.len + range_t.type]
		jne sr_1

; ...check if this range adjacent to previous range
		push dx
		push ax
			mov ax,[si - range_t.len + range_t.adr + 0]
			mov dx,[si - range_t.len + range_t.adr + 2]
			add ax,[si - range_t.len + range_t.size + 0]
			adc dx,[si - range_t.len + range_t.size + 2]
			xor ax,bx	; use XOR to compare for equality
			xor dx,cx
			or ax,dx
		pop ax
		pop dx
		jne sr_1

; ...coalesce this range with previous range by adding size of this
; range to previous range
		add [si - range_t.len + range_t.size + 0],ax
		adc [si - range_t.len + range_t.size + 2],dx
		jmp short sr_2
sr_1:
		mov [si + range_t.adr + 0],bx
		mov [si + range_t.adr + 2],cx
		mov [si + range_t.size + 0],ax
		mov [si + range_t.size + 2],dx
		mov [si + range_t.type],di
		inc byte [g_num_ranges]

; return CY=0 for success
		clc
sr_2:
	pop si
	ret
sr_3:
		;mov si,_many_ranges_msg
		;call puts
	pop si

; return CY=1 for error
	stc
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:		store_range_1
; action:	same as store_range, but sets memory range type = 1
; in:		CX:BX=linear base address of range,
;		DX:AX=size of range, DI=type of range
;		[g_num_ranges] set
; out:		(nothing)
; modifies:	[g_num_ranges], one entry at [g_ranges]
; minimum CPU:	8088
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

store_range_1:
	push di
		mov di,1
		call store_range
	pop di
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:		store_range_1m
; action:	same as store_range, but sets memory range type = 1,
;		sets base address to 1 meg, and size is in K
; in:		AX=size of range, in Kbytes
;		[g_num_ranges] set
; out:		(nothing)
; modifies:	[g_num_ranges], one entry at [g_ranges]
; minimum CPU:	8088
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

store_range_1m:
	push dx
	push cx
	push bx

; set range base to 1 meg
		xor bx,bx
		mov cx,10h

; convert from K in AX to bytes in DX:AX
		mov dx,1024
		mul dx
		call store_range_1
	pop bx
	pop cx
	pop dx
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:		extmem_e820
; action:	gets extended memory info using INT 15h AX=E820h
; in:		(nothing)
; out (error):	CY=1
; out (success):CY=0
; modifies:	[g_num_ranges], entries at [g_ranges]
; minimum CPU:	386
; notes: comments on BIOS bugs from b-15E820 of Ralf Brown's list and from
;	http://marc.theaimsgroup.com/?l=linux-kernel&m=99322719013363&w=2
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

extmem_e820:
	push es
	push di
	push edx
	push ecx
	push ebx
	push eax
		push ds
		pop es
		mov di,_buf1
		xor ebx,ebx		; INT 15h AX=E820h continuation value
		mov edx,534D4150h	; 'SMAP'
		mov ecx,20		; 20-byte buffer
		mov eax,0000E820h
		int 15h

; CY=1 on first call to INT 15h AX=E820h is an error
		jc em_e820_err
em_e820_loop:
; return EAX other than 'SMAP' is an error
		cmp eax,534D4150h
		stc
		jne em_e820_err
		push bx
			mov bx,[es:di + 0] ; base
			mov cx,[es:di + 2]
			mov ax,[es:di + 8] ; size
			mov dx,[es:di + 10]
			mov di,[es:di + 16]; type
			call store_range
		pop bx

; exit now if too many memory ranges
		jc em_e820_ok
		or ebx,ebx
		je em_e820_ok

; "In addition the SMAP signature is restored each call, although not
;  required by the specification in order to handle some known BIOS bugs."
		mov edx,534D4150h	; 'SMAP'
		mov ecx,20		; 20-byte buffer
		mov eax,0000E820h
		int 15h

; "the BIOS is permitted to return a nonzero continuation value in EBX
;  and indicate that the end of the list has already been reached by
;  returning with CF set on the next iteration"
		jnc em_e820_loop
em_e820_ok:
		clc
em_e820_err:
	pop eax
	pop ebx
	pop ecx
	pop edx
	pop di
	pop es
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:		extmem_e801
; action:	gets extended memory size using INT 15h AX=E801h
; in:		(nothing)
; out (error):	CY=1
; out (success):CY=0
; modifies:	[g_num_ranges], entries at [g_ranges]
; minimum CPU:	286
; notes:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

extmem_e801:
	push dx
	push cx
	push bx
	push ax
		mov ax,0E801h

; "...the INT 15 AX=0xE801 service is called and the results are sanity
;  checked. In particular the code zeroes the CX/DX return values in order
;  to detect BIOS implementations that do not set the usable memory data.
;  It also handles older BIOSes that return AX/BX but not AX/BX data." [sic]
		xor dx,dx
		xor cx,cx
		int 15h
		jc em_e801_2
		push ax
			or ax,bx
		pop ax
		jne em_e801_1
		mov ax,cx
		mov bx,dx
em_e801_1:
		push bx

; convert from Kbytes in AX to bytes in DX:AX,
; set range base to 1 meg and store it
			call store_range_1m

; convert stacked value from 64K-blocks to bytes in DX:AX
		pop dx
		xor ax,ax

; set range base to 16 meg and display it
		xor bx,bx
		mov cx,100h
		call store_range_1
		clc
em_e801_2:
	pop ax
	pop bx
	pop cx
	pop dx
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:		extmem_88
; action:	gets extended memory size using INT 15h AH=88h
; in:		(nothing)
; out (error):	CY=1
; out (success):CY=0
; modifies:	[g_num_ranges], one entry at [g_ranges]
; minimum CPU:	286
; notes:	HIMEM.SYS will hook this interrupt and make it return 0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

extmem_88:
	push dx
	push cx
	push bx
	push ax
		mov ax,8855h
		int 15h

; "not all BIOSes correctly return the carry flag, making this call
;  unreliable unless one first checks whether it is supported through
;  a mechanism other than calling the function and testing CF"
;
; test if AL register modified by INT 15h AH=88h
		cmp al,55h
		jne em_88_1
		mov ax,88AAh
		int 15h
		cmp al,0AAh
		stc
		je em_88_2
em_88_1:
; if this call returns 0, it could mean zero extended memory,
; but it's much more likely that HIMEM.SYS has hooked this interrupt
; Return CY=1
		or ax,ax
		stc
		je em_88_2

; convert from Kbytes in AX to bytes in DX:AX,
; set range base to 1 meg and store it
		call store_range_1m
		clc
em_88_2:
	pop ax
	pop bx
	pop cx
	pop dx
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:		get_memory_map
; action:	gets map (linear base address:size pairs) of
;		available conventoinal and extended memory
; in:		[g_cpu32] set
; out:		[g_num_ranges], entries at [g_ranges]
; modifies:	(nothing)
; minimum CPU:	8088
; notes:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

get_memory_map:
	push dx
	push cx
	push bx
	push ax

; INT 15h AX=E820h works only with 32-bit CPUs
		mov byte [g_num_ranges],0
		test byte [g_cpu32],0FFh
		je gmm_1
		call extmem_e820
		jnc gmm_2
gmm_1:
; before trying other BIOS calls, use INT 12h to get conventional memory size
		mov byte [g_num_ranges],0
		int 12h

; convert from K in AX to bytes in DX:AX
		mov dx,1024
		mul dx

; set range base to 0 and store it
		xor bx,bx
		xor cx,cx
		call store_range_1

; try INT 15h AX=E801h
		call extmem_e801
		jnc gmm_2

; try INT 15h AH=88h
		call extmem_88
		jnc gmm_2

; get extended memory size from CMOS
		in al,70h
		and al,80h	; b7 of port 70h controls NMI -- don't touch
		mov dl,al

		mov al,dl	; extended memory size (K) in CMOS...
		or al,18h	; ...registers 18h (MSB)...
		out 70h,al
		in al,71h
		mov dh,al

		mov al,dl
		or al,17h	; ...and 17h (LSB)
		out 70h,al
		in al,71h
		mov dl,al

		mov ax,dx

; convert from Kbytes in AX to bytes in DX:AX,
; set range base to 1 meg and store it
		call store_range_1m
gmm_2:
	pop ax
	pop bx
	pop cx
	pop dx
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; XMS CODE
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:		xms_init
; action:	allocates largest available block of free XMS memory
;		and locks it
; in:		[_xms_entry] set
; out (error):	[g_extmem_size] remains = 0
; out (success):[g_extmem_size], [g_extmem_adr, [_xms_handle] set
; modifies:	(nothing)
; minimum CPU:	'286
; notes:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

xms_init:
	push dx
	push cx
	push bx
	push ax

; call XMS function 08h: "Query free extended memory"
; returns size of largest block in AX (Kbytes)
		mov ah,08h
		mov bl,0
		call far [_xms_entry]

; save XMS size (in K) in CX
		mov cx,ax

; call XMS function 09h: "Allocate extended memory block" of size DX (Kbytes)
; returns status in AX (must be 1) and handle in DX
		mov dx,ax
		mov ah,09h
		call far [_xms_entry]
		cmp ax,1
		jne ax_3
		mov [_xms_handle],dx

; call XMS function 0Ch: "Lock extended memory block" with handle DX
; returns status in AX (must be 1) and linear address of block in DX:BX
;
; This operation will fail in a Windows 9x DOS box and cause a dialog
; to pop up, suggesting "MS-DOS mode"
		mov ah,0Ch
		call far [_xms_entry]
		cmp ax,1
		jne ax_2
		mov [_xms_in_use],al

; convert size from K to bytes and store it
		mov ax,1024
		push dx
			mul cx
			mov [g_extmem_size + 0],ax
			mov [g_extmem_size + 2],dx
		pop dx

; copy linear address to AX:CX
		mov ax,dx
		mov cx,bx

; round linear address up to next 4096-byte boundary...
		add bx,4095
		adc dx,byte 0
		and bx,0F000h
		mov [g_extmem_adr + 0],bx
		mov [g_extmem_adr + 2],dx

; ...and round down block size because linear address was rounded up
		sub cx,bx
		sbb ax,dx
		add [g_extmem_size + 0],cx
		adc [g_extmem_size + 2],ax

; round down block size to multiple of 4K
		and word [g_extmem_size + 0],0F000h

; return CY=0 for success
		clc
		jmp short ax_4
ax_2:
; call XMS function 0Ah: "Free extended memory block"
		mov dx,[_xms_handle]
		mov ah,0Ah
		call far [_xms_entry]
ax_3:
; return CY=1 for error
		stc
ax_4:
	pop ax
	pop bx
	pop cx
	pop dx
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:		xms_exit
; action:	cleans up DOS environment after failed/aborted load
; in:		(nothing)
; out:		(nothing)
; modifies:	(nothing)
; minimum CPU:	8088
; notes:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

EXP xms_exit
	push dx
	push ax

; free XMS memory in use
		xor ax,ax
		or al,[_xms_in_use]
		je eg_1

		mov dx,[_xms_handle]
		mov ah,0Dh		; "Unlock extended memory block"
		call far [_xms_entry]
		mov dx,[_xms_handle]	; "Free extended memory block"
		mov ah,0Ah
		call far [_xms_entry]
eg_1:
	pop ax
	pop dx
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:		enable_a20_xms
; action:	uses XMS to enable A20 gate
; in:		[g_xms], [_xms_entry] set
; out (failure):AX=0
; out (success):AX=1
; modifies:	(nothing)
; minimum CPU:	'286
; notes:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

enable_a20_xms:
	test byte [g_xms],0FFh
	je eax_1

; call XMS function 03h: "Global enable A20, for using the HMA"
; returns status in AX (must be 1)
	mov ah,3
	call far [_xms_entry]
eax_1:
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CODE TO ENABLE A20 GATE
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:		enable_a20_ps2
; action:	enables A20 line using INT 15h AX=2401h
; in:		(nothing)
; out:		(nothing)
; modifies:	(nothing)
; minimum CPU:	'286
; notes:	I don't think this control method is very common, but it
;		should be safe to try even if it's not supported.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

enable_a20_ps2:
	push ax
		mov ax,2401h	; mov ax,2400h to disable A20
		int 15h
	pop ax
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:		enable_a20_fast
; action:	enables A20 line using "port 92" ("fast") method
; in:		(nothing)
; out:		(nothing)
; modifies:	(nothing)
; minimum CPU:	'286
; notes:	use INT 15h AX=2403h to test if this method supported
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

enable_a20_fast:
	push ax
		in al,92h
		or al,2		; AND ~2 to disable A20
		out 92h,al
	pop ax
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:		enable_a20_at
; action:	enables A20 line using "AT" method
; in:		(nothing)
; out:		(nothing)
; modifies:	(nothing)
; minimum CPU:	'286
; notes:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

kbd0:
	jmp short $+2	; a delay (probably not effective nor necessary)
	in al,60h	; read and discard data/status from 8042
kbd:
	jmp short $+2	; delay
	in al,64h
	test al,1	; output buffer (data _from_ keyboard) full?
	jnz kbd0	; yes, read and discard
	test al,2	; input buffer (data _to_ keyboard) empty?
	jnz kbd		; no, loop
	ret

enable_a20_at:
	push ax
	pushf

; Yay, feedback! Chase told me it works better if I shut off interrupts:
		cli
		call kbd
		mov al,0D0h	; 8042 command byte to read output port
		out 64h,al
eaa_1:
		in al,64h
		test al,1	; output buffer (data _from_ keyboard) full?
		jz eaa_1	; no, loop

		in al,60h	; read output port
		or al,2		; AND ~2 to disable A20
		mov ah,al

		call kbd
		mov al,0D1h	; 8042 command byte to write output port
		out 64h,al

		call kbd
		mov al,ah	; the value to write
		out 60h,al

		call kbd
	popf
	pop ax
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:		enable_a20_vectra
; action:	enables A20 line using "Vectra" method
; in:		(nothing)
; out:		(nothing)
; modifies:	(nothing)
; minimum CPU:	'286
; notes:	this method makes old versions of Bochs crash
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

enable_a20_vectra:
	push ax
	pushf
		cli
		call kbd
		mov al,0DFh	; mov al,0DDh to disable A20
		out 64h,al
		call kbd
	popf
	pop ax
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:		verify_a20
; action:	checks if A20 line enabled or disabled
; in:		(nothing)
; out (A20 disabled):	ZF=1
; out (A20 enabled):	ZF=0
; modifies:	(nothing)
; minimum CPU:	'286
; notes:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

verify_a20:
	push ax
	push ds
	push es
		xor ax,ax
		mov ds,ax
		dec ax
		mov es,ax

		mov ax,[es:10h]		; read word at FFFF:0010 (1 meg)
		not ax			; 1's complement
		push word [0]		; save word at 0000:0000 (0)
			mov [0],ax	; word at 0 = ~(word at 1 meg)
			mov ax,[0]	; read it back
			cmp ax,[es:10h]	; fail if word at 0 == word at 1 meg
		pop word [0]
	pop es
	pop ds
	pop ax
	ret		; if ZF=1, the A20 gate is NOT enabled


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:		enable_a20
; action:	enables A20 and verifies that it's enabled
; in:		[g_xms] must be set
; out (A20 disabled):	ZF=1
; out (A20 enabled):	ZF=0
; modifies:	(nothing)
; minimum CPU:	'286
; notes:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

enable_a20:
	push bx
	push ax

; first, check if A20 already on e.g. turned on by the BIOS,
; or maybe a system that has no A20 gate
		call verify_a20
		jne ea_3

; if XMS is present, use it to control A20
		test byte [g_xms],0FFh
		je ea_1
		call enable_a20_xms
		or ax,ax
		jmp short ea_3
ea_1:
; try PS/2 method first
		call enable_a20_ps2
		call verify_a20
		jne ea_3

; check if A20 controlled by 8042 keyboard controller or I/O port 92h
		mov ax,2403h
		int 15h
		jc ea_2

; if "fast" (port 92h) method supported, try it
		test bl,2
		je ea_2
		call enable_a20_fast
		call verify_a20
		jne ea_3
ea_2:
; try "AT" method
		call enable_a20_at
		call verify_a20
		jne ea_3

; try "Vectra" method
; do this last, because it makes Bochs panic
		call enable_a20_vectra
		call verify_a20
ea_3:
	pop ax
	pop bx
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CODE TO COPY STUFF TO EXTENDED MEMORY
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:		copy_pmode
; action:	copies data from one linear address to another,
;		using 'unreal mode'
; in:		BX:SI = source linear address, DX:AX = byte count,
;		CX:DI = destination linear address
;		[g_v86] must be set
; out (error):	AX != 0
; out (success):AX == 0
; modifies:	AX and the top 16 bits of some 32-bit registers
; minimum CPU:	'386
; notes:	want to support '286 CPU or not?
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

copy_pmode:
	push es
	push ds
	push dx	; push count
	push ax
	push cx	; push dst adr
	push di
	push bx	; push src adr
	push si

; make sure we're not in V86 mode
		xor ax,ax
		or al,[g_v86]
		jne cp_4

; put LINEAR (non-segmented) address of GDT into _c_gdt_ptr
		mov ax,ds
		mov dx,16
		mul dx
		add ax,_c_gdt
		adc dx,byte 0
		mov [_c_gdt_ptr + 2],ax
		mov [_c_gdt_ptr + 4],dx
; interrupts off
		cli
; enable A20 gate
		call enable_a20
		mov ax,-1
		je cp_4
; load GDT
		o32 lgdt [_c_gdt_ptr]
; begin switch to pmode
		mov eax,cr0
		or al,1
		mov cr0,eax

; load DS and ES with selectors to pmode data segment descriptor
; with limit 4Gbytes - 1
		mov bx,C_BOOT_DS
		mov ds,bx
		mov es,bx

; go back to (un)real mode
		and al,0FEh
		mov cr0,eax

; set segment register (and base address) = 0
; The 4Gbyte limit remains in the descriptor caches
		xor ax,ax
		mov es,ax
		mov ds,ax

; pop source address into ESI; dest address into EDI; count into ECX
		pop esi
		pop edi
		pop ecx
		push ecx	; restore stack layout
		push edi
		push esi
; if src < dst...
		cld
		cmp edi,esi
		jb cp_3

; ...set EFLAGS.DF and adjust registers to copy backwards
		std
		add esi,ecx
		dec esi
		add edi,ecx
		dec edi
cp_3:
		a32 rep movsb
		cld
		xor ax,ax
cp_4:
	pop si
	pop bx
	pop di
	pop cx
	add sp,byte 2	; pop ax
	pop dx
	pop ds
	pop es
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:		copy_int15
; action:	copies data from one linear address to another,
;		using INT 15h AH=87h
; in:		BX:SI = source linear address, DX:AX = byte count,
;		CX:DI = destination linear address
; out (error):	AX != 0
; out (success):AX == 0
; modifies:	AX
; minimum CPU:	'286
; notes:	### - can INT 15h AH=87h handle overlapping src and dst?
;		HIMEM.SYS will hook this interrupt
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

copy_int15:
	push es
	push si
	push dx
	push cx
	push bx

; set up GDT for INT 15h AH=87h
		mov [_gdt_src + 2],si
		mov [_gdt_src + 4],bl
		mov [_gdt_src + 7],bh

		mov [_gdt_dst + 2],di
		mov [_gdt_dst + 4],cl
		mov [_gdt_dst + 7],ch

; convert count from bytes to words
; xxx - this rounds down
		shr dx,1
		rcr ax,1
; point to GDT
		mov si,ds
		mov es,si
		mov si,_c_gdt
		jmp short ci_3
ci_1:
; figure out word count for this loop
		mov cx,8000h
		or dx,dx
		jne ci_2
		cmp ax,cx
		ja ci_2
		mov cx,ax
ci_2:
; copy up to 8000h words (65536 bytes)
		push ax
			mov ah,87h
			int 15h
		pop ax
		jc ci_4

; decrement total word count
		sub ax,cx
		sbb dx,byte 0

; convert count from words back to bytes...
		shl ax,1
		rcl dx,1

; ...advance GDT pointers...
		add [_gdt_src + 2],ax
		adc [_gdt_src + 4],dl
		adc [_gdt_src + 7],dh

		add [_gdt_dst + 2],ax
		adc [_gdt_dst + 4],dl
		adc [_gdt_dst + 7],dh

; ...and convert count back to words
		shr dx,1
		rcr ax,1
ci_3:
; exit if count=0
		push ax
			or ax,dx
		pop ax
		jne ci_1
		jmp short ci_5
ci_4:
		mov ax,-1
ci_5:
	pop bx
	pop cx
	pop dx
	pop si
	pop es
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:		non_xms_copy
; action:	copies from conventional/extended memory
;		to extended memory, using either INT 15h AH=87h
;		or a built-in pmode copy function
; in:		BX:SI = source linear address, DX:AX = byte count,
;		CX:DI = destination linear address
; out (error):	AX != 0
; out (success):AX == 0
; modifies:	AX
; minimum CPU:	'386 (for copy_pmode; copy_int15 needs only '286)
; notes:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

non_xms_copy:
; xxx - what's up with this? why doesn't it work?
	;call copy_int15
	;or ax,ax
	;je nxc_1
	call copy_pmode
nxc_1:
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:		copy_linear
; action:	copies data between two linear addresses
;		(conventional or extended memory)
; in:		args on stack per C prototype
; out (error):	AX != 0
; out (success):AX == 0
; modifies:	AX
; minimum CPU:	'286
; notes:	C prototype: void copy_linear(long dst_linear,
;				long src_linear, unsigned count);
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; I had code here to copy stuff to extended memory using XMS but,
; since the XMS memory block is locked, it's safe (and a LOT simpler)
; to use INT 15h AH=87h or 'unreal mode' to copy to/from the block

EXP copy_linear
	push bp
		mov bp,sp
		push di
		push si
		push dx
		push cx
		push bx
			mov di,[bp + 4]	; CX:DI=dst linear adr
			mov cx,[bp + 6]

			mov si,[bp + 8]	; BX:SI=src linear adr
			mov bx,[bp + 10]

			mov ax,[bp + 12]; DX:AX=byte count
			xor dx,dx

			call non_xms_copy
		pop bx
		pop cx
		pop dx
		pop si
		pop di
	pop bp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CODE TO ENTER PROTECTED MODE
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:		pmode_int15
; action:	enables 32-bit protected mode using INT 15h AH=89h
;		and jumps to kernel
; in:		[_real_ds] and [g_entry] must be set
; out (error):	(nothing)
; out (success):(does not return)
; modifies:	(nothing)
; minimum CPU:	'386
; notes:	returns only if error
;		want to support '286 CPU or not?
;		I assume INT 15h AH=89h turns on the A20 gate
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

pmode_int15:
	push es
	push si
	push dx
	push bx
	push ax

; set up GDT
		mov ax,ds
		mov dx,16
		mul dx
; BX:SI=DX:AX
		mov si,ax
		mov bx,dx

; put LINEAR (non-segmented) address of GDT into _p_gdt_ptr
		add ax,_p_gdt
		adc dx,byte 0
		mov [_p_gdt_ptr + 2],ax
		mov [_p_gdt_ptr + 4],dx

; put LINEAR (non-segmented) address of IDT into _idt_ptr
		mov ax,si
		mov dx,bx
		add ax,_idt
		adc dx,byte 0
		mov [_idt_ptr + 2],ax
		mov [_idt_ptr + 4],dx

; set protected-mode segment addresses = 16 * real-mode segment register value
; this gives everything the same address in real mode and pmode
		mov [_gdt_ds + 2],si
		mov [_gdt_ds + 4],bl
		mov [_gdt_ds + 7],bh

; ...except for ES segment, which we'll leave linear (base address = 0)
		;mov [_gdt_es + 2],si
		;mov [_gdt_es + 4],bl
		;mov [_gdt_es + 7],bh

		mov [_gdt_ss + 2],si
		mov [_gdt_ss + 4],bl
		mov [_gdt_ss + 7],bh

		mov ax,cs
		mov dx,16
		mul dx
		mov [_gdt_cs + 2],ax
		mov [_gdt_cs + 4],dl
		mov [_gdt_cs + 7],dh

; use INT 15h AH=89h to switch from real mode to pmode
		mov si,ds
		mov es,si
		mov si,_p_gdt
		mov bx,2820h	; IRQ-to-INT mappings for 8259 chips
		mov ah,89h
		cli
		int 15h
		jnc pmode	; short jump; identical in 16- or 32-bit mode
pi_err:
	pop ax
	pop bx
	pop dx
	pop si
	pop es
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 32-bit code
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	BITS 32

pmode:
; clear the damn NT bit and set IOPL=0...
	push dword 2
	popfd

o32 lidt [_idt_ptr]
xor eax,eax
mov cr3,eax


; eat stray keypresses to prevent keyboard freezing up
	in al,60h

; set up Multiboot
	movzx ebx,word [_real_ds]
	shl ebx,4
	add ebx,g_mboot
	mov eax,2BADB002h

; load data segment registers with linear selectors
	mov cx,LINEAR_DS
	mov ds,cx
	mov ss,cx
	mov es,cx
	mov fs,cx
	mov gs,cx
	;mov ds,ecx	; one byte smaller than "mov ds,cx"
	;mov ss,ecx
	;mov es,ecx
	;mov fs,ecx
	;mov gs,ecx

; ### - pmode stack at linear address 64K -- make this an EQU
	mov esp,10000h

	xor ebp,ebp
	xor edi,edi
	xor esi,esi
	xor ecx,ecx
	xor edx,edx

; jmp LINEAR_CODE_SEL:dword 0
; 1-byte JMP at [entry+0]:	EA
; 4-byte offset at [entry+1]:	00 00 00 00
; 2-byte selector at [entry+5]:	08 00
	db 0EAh

EXP g_entry
	dd 0
	dw LINEAR_CS

; handler for CPU exceptions that occur during/after INT 15h AH=89h
unhand:
	mov ax,LINEAR_DS
	mov ds,ax

; put white-on-blue '!' in upper left corner of screen, then freeze
	mov word [dword 0B8000h],9F21h
	jmp $

	BITS 16

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:		pmode_raw
; action:	enables protected mode using "raw" method
;		and jumps to kernel
; in:		[_real_ds] and [g_entry] must be set
; out (error):	(nothing)
; out (success):(does not return)
; modifies:	(nothing)
; minimum CPU:	'386
; notes:	returns only if error
;		want to support '286 CPU or not?
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

pmode_raw:
	push dx
	push bx
	push ax

; set up GDT
		mov ax,ds
		mov dx,16
		mul dx
; BX:SI=DX:AX
		mov si,ax
		mov bx,dx

; put LINEAR (non-segmented) address of GDT into _p_gdt_ptr
		add ax,_p_gdt
		adc dx,byte 0
		mov [_p_gdt_ptr + 2],ax
		mov [_p_gdt_ptr + 4],dx

; set protected-mode segment addresses = 16 * real-mode segment register value
; this gives everything the same address in real mode and pmode
		mov [_gdt_ds + 2],si
		mov [_gdt_ds + 4],bl
		mov [_gdt_ds + 7],bh

		mov ax,cs
		mov dx,16
		mul dx
		mov [_gdt_cs + 2],ax
		mov [_gdt_cs + 4],dl
		mov [_gdt_cs + 7],dh
; interrupts off
		cli
; enable A20 gate
		call enable_a20
		je pr_err
; load GDT
		o32 lgdt [_p_gdt_ptr]
; switch to pmode
 ;		mov eax,cr0
 ;		or al,1
 mov eax,1
		mov cr0,eax
		mov ax,P_BOOT_DS
		mov ds,ax
		mov ss,ax
		mov es,ax
		mov fs,ax
		mov gs,ax
		jmp BOOT_CS:pmode
pr_err:
	pop ax
	pop bx
	pop dx
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:		enter_pmode
; action:	enables protected mode and jumps to kernel
; in:		(args as shown in C prototype)
; out (error):	(nothing)
; out (success):(does not return)
; modifies:	(nothing)
; minimum CPU:	'286
; notes:	C prototype: void enter_pmode(void);
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

EXP enter_pmode
	push es
	push di
	push si
	push dx
	push cx
	push bx
	push ax

; if kernel (at g_linear) is not where it wants to be (at g_phys),
; copy it there now, trampling on used XMS (including DOS) if necessary
		mov si,[g_linear + 0]
		mov bx,[g_linear + 2]
		mov di,[g_phys + 0]
		mov cx,[g_phys + 2]

		mov ax,si
		xor ax,di
		jne ep_1
		mov ax,bx
		xor ax,cx
		je ep_4
ep_1:
; g_linear (where kernel is currently loaded) is start of USED extended mem,
; g_extmem_adr is start of FREE extended memory -- so the difference is
; the size of kernel + modules
		mov ax,[g_extmem_adr + 0]
		mov dx,[g_extmem_adr + 2]
		sub ax,[g_linear + 0]
		sbb dx,[g_linear + 2]
		call non_xms_copy

; fix up module start and end addresses in g_mods[]
		mov ax,[g_phys + 0]
		mov dx,[g_phys + 2]
		sub ax,[g_linear + 0]
		sbb dx,[g_linear + 2]
IMP g_mods
		mov bx,g_mods
		mov cx,[g_mboot + 20]	; g_mboot.num_mods
		or cx,cx
		je ep_3
ep_2:
		add [bx + 0],ax		; g_mods[n].start_adr
		adc [bx + 2],dx
		add [bx + 4],ax		; g_mods[n].end_adr
		adc [bx + 6],dx
		add bx,16		; sizeof(mboot_mod_t)
		loop ep_2
ep_3:
ep_4:
		cli
; clts

; try using INT 15h AH=89h to enter pmode
 ;		call pmode_int15

; that didn't work, try 'raw' method of entering pmode
		call pmode_raw
	pop ax
	pop bx
	pop cx
	pop dx
	pop si
	pop di
	pop es
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; DATA
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

SEGMENT _DATA

; Interrupt Descriptor Table (IDT) used only by INT 15h AH=89h (pmode_int15)
_idt:
	%rep 32

	dw unhand	; low 16 bits of ISR offset (unhand & 0FFFFh)
	dw BOOT_CS	; selector
	db 0
	db 8Eh		; present,ring 0,32-bit interrupt gate
	dw 0		; high 16 bits of ISR offset (unhand / 65536)

	%endrep
_idt_end:

; Global Descriptor Table (GDT) for entering 'P'mode
; descriptor #0: NULL descriptor
_p_gdt:
	dd 0, 0

; descriptor #1: GDT "pseudo-descriptor" used by INT 15h AH=89h (pmode_int15)
; also used by pmode_raw
_p_gdt_ptr:
	dw _p_gdt_end - _p_gdt - 1 ; GDT limit
	dd _p_gdt		; linear adr of GDT (set in pmode_int15)
	dw 0

; descriptor #2: IDT "pseudo-descriptor" used by INT 15h AH=89h (pmode_int15)
_idt_ptr:
	dw _idt_end - _idt - 1	; IDT limit
	dd _idt			; linear adr of IDT (set in pmode_int15)
	dw 0

; descriptor #3: DS used by INT 15h AH=89h (pmode_int15)
; also used by pmode_raw
P_BOOT_DS equ $-_p_gdt
_gdt_ds:
	dw 0FFFFh	; 20-bit limit = 0FFFFFh (in pages; =4Gbytes - 1)
	dw 0		; 32-bit base address = 0
	db 0
	db 93h		; present,ring0,data,expand-up,writable,accessed
	db 0CFh		; page-granular limit,32-bit segment
	db 0

; descriptor #4: ES used by INT 15h AH=89h (pmode_int15)
LINEAR_DS equ $-_p_gdt
_gdt_es:
	dw 0FFFFh	; 20-bit limit = 0FFFFFh (in pages; =4Gbytes - 1)
	dw 0		; 32-bit base address = 0
	db 0
	db 93h		; present,ring0,data,expand-up,writable,accessed
	db 0CFh		; page-granular limit,32-bit segment
	db 0

; descriptor #5: SS used by INT 15h AH=89h (pmode_int15)
_gdt_ss:
	dw 0FFFFh	; 20-bit limit = 0FFFFFh (in pages; =4Gbytes - 1)
	dw 0		; 32-bit base address = 0
	db 0
	db 93h		; present,ring0,data,expand-up,writable,accessed
	db 0CFh		; page-granular limit,32-bit segment
	db 0

; descriptor #6: CS used by INT 15h AH=89h (pmode_int15)
; also used by pmode_raw
BOOT_CS equ $-_p_gdt
_gdt_cs:
	dw 0FFFFh	; 20-bit limit = 0FFFFFh (in pages; =4Gbytes - 1)
	dw 0		; 32-bit base address = 0
	db 0
	db 9Bh		; present,ring0,code,non-conforming,readable,accessed
	db 0CFh		; page-granular limit,32-bit segment
	db 0

; descriptor #7: used by INT 15h AH=89h (pmode_int15)
	dd 0, 0

; descriptor #8: linear CS (base adr = 0); used to jump to kernel
LINEAR_CS equ $-_p_gdt
	dw 0FFFFh	; 20-bit limit = 0FFFFFh (in pages; =4Gbytes - 1)
	dw 0		; 32-bit base address = 0
	db 0
	db 9Bh		; present,ring0,code,non-conforming,readable,accessed
	db 0CFh		; page-granular limit,32-bit segment
	db 0

_p_gdt_end:

; Global Descriptor Table (GDT) for 'C'opying
; descriptor #0: NULL descriptor
_c_gdt:
	dd 0, 0

; descriptor #1: used by INT 15h AH=87h (copy_int15)
	dd 0, 0

; descriptor #2: source data segment used by INT 15h AH=87h (copy_int15)
C_BOOT_DS equ $-_c_gdt	; also used by copy_pmode
_gdt_src:
	dw 0FFFFh	; 20-bit limit = 0FFFFFh (in pages; =4Gbytes - 1)
	dw 0		; 32-bit base address = 0
	db 0
	db 93h		; present,ring0,data,expand-up,writable,accessed
	db 0CFh		; page-granular limit,32-bit segment
	db 0

; descriptor #3: destination data segment used by INT 15h AH=87h (copy_int15)
_gdt_dst:
	dw 0FFFFh	; 20-bit limit = 0FFFFFh (in pages; =4Gbytes - 1)
	dw 0		; 32-bit base address = 0
	db 0
	db 93h		; present,ring0,data,expand-up,writable,accessed
	db 0CFh		; page-granular limit,32-bit segment
	db 0

; descriptor #4: used by INT 15h AH=87h (copy_int15)
	dd 0, 0

; descriptor #5: used by INT 15h AH=87h (copy_int15)
	dd 0, 0
_c_gdt_end:

_c_gdt_ptr:		; GDT "pseudo-descriptor"
	dw _c_gdt_end - _c_gdt - 1 ; GDT limit
	dd _c_gdt	; linear adr of GDT (set in copy_pmode)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; BSS
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

SEGMENT _BSS

; CPU, system, and RAM info
EXP g_cpu32
	resb 1
EXP g_dos
	resb 1
EXP g_xms
	resb 1
EXP g_v86
	resb 1
_real_ds:
	resw 1
; XMS info
_xms_in_use:
	resb 1
_xms_entry:
	resw 2
_xms_handle:
	resw 1
; low (conventional) and high (extended) memory heaps
EXP g_convmem_size
	resd 1
EXP g_convmem_adr
	resd 1
EXP g_extmem_size
	resd 1
EXP g_extmem_adr
	resd 1

; used for INT 15h AX=E820h (20 bytes)
_buf1:
	resb 20

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; INFO PASSED TO THE KERNEL
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	ALIGN 16

; BIOS memory ranges
EXP g_num_ranges	; up to MAX_RANGES
	resb 1
EXP g_ranges
	resb (MAX_RANGES * range_t.len)

