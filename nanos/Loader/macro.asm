%macro new_Page 1
	push	eax
	call	check_edi
	pop	eax
	pusha
	mov	eax, %1
	shl	eax, 12
	call	PD_add_Page
	popa
%endmacro

%macro new_user_Page 1
	push	eax
	call	check_edi
	pop	eax
	pusha
	mov	eax, %1
	shl	eax, 12
	or	edi, 0111b
	call	PD_add_Page
	popa
%endmacro

%macro pt_fill 1
    mov  eax, %1
    rep  stosd
%endmacro

%macro put 0
    stosd
    dec	ecx
%endmacro

%macro put_fill 3
    ;Copy %1 bytes from %2 to edi
    ;Fill up to %3 bytes with zeros
    mov  ecx, (%1)/4
    mov  esi, %2
    rep  movsd
    
    %if ((%3)/4 - (%1)/4) > 0
         mov  ecx, ((%3) - (%1))/4
         mov  eax, 0
         rep  stosd
    %endif
%endmacro


;Descriptors, ONLY to be used in mem_gdt.asm and mem_idt.asm

%macro DataDesc 3
;Base(pages), Limit(pages), Settings
;Settings-Data = "PDp10EWA" Present Dpl Expand-down Writeable Accessed
;Settings-Code = "PDp11CRA" Present Dpl Conforming Readable Accessed
    mov  eax, ((%1 << 12) & 0FFFFh) << 10h | ((%2) & 0FFFFh)
    put
    mov  eax, ((((%1 << 12) >> 10h) & 0FF00h) | 0C0h | ((%2 >> 10h) & 0Fh)) << 10h | (((%1 << 12) >> 10h) & 0FFh) | ((%3 & 0FFh) << 8)
    put
%endmacro

%macro GateDesc 3
;Selector, Offset, Settings
;Settings-Data = "PDp0Type" Present Dpl Type
;Type: 1100=Call gate, 1110=Interrupt, 1111=Trap gate
    mov  eax, (%1 & 0FFFFh) << 10h | (%2 & 0FFFFh)
    put
    mov  eax, (%2 & 0FFFF0000h) | ((%3 & 0FFh) << 8)
    put
%endmacro

%macro TSSDesc 3
;Base(bytes), Limit(bytes), Settings
;Settings-Data = "PDp010B1" Present Dpl Busy
    mov  eax, ((%1) & 0FFFFh) << 10h | ((%2) & 0FFFFh)
    put
    mov  eax, ((((%1) >> 10h) & 0FF00h) | 0C0h | (((%2) >> 10h) & 0Fh)) << 10h | ((%1 >> 10h) & 0FFh) | ((%3 & 0FFh) << 8)
    put
%endmacro