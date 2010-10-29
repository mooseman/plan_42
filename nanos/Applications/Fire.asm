; Information
;
;  Program Title      	: NASM-IDE Fire Code Demo
;  External name 	: FIRE.COM
;  Version       		: 1.0
;  Start date    		: 15/10/1997
;  Last update  		: 15/10/1997
;  Author       		: Rob Anderton
;  Description   		: An example of a flickering fire effect programmed using
;                  		  NasmEdit 1.0 and NASM 0.98.
;
;                  		  Based on code by Denthor of Asphyxia (written using TASM).


[BITS 16]	 ; Set 16 bit code generation
[ORG 0x0100]   	; Set code start address to 100h (COM file)

[SECTION .data]              ; Data section (initialised variables)

FireSeed db $1234            ; Random number seed

%include "FIREPAL.INC"       ; Include 256 colour palette RGB data

; Text message displayed at the end of the demo
EndMessage db 'Fire demonstration for NASM-IDE 1.1.', 13, 10, '$'

[SECTION .bss]               ; BSS section (unitialised variables)

FireScreen resb $2300        ; Virtual screen buffer


[SECTION .text]              ; Text section (the code to be assembled)

    jmp       START          ; Jump to main code section


FIRE_INIT:                   ; Initialise 320x200 X mode

    mov       ax, $0013      ; Set MCGA 320x200x256 mode
    int       $10

    mov       ax, $0A000
    mov       es, ax
    xor       di, di         ; ES:DI points to top left of screen

    cli
    cld
    mov       dx, $03C4
    mov       ax, $0604      ; Enter unchained mode
    out       dx, ax

    mov       ax, $0F02      ; Enable all planes
    out       dx, ax

    xor       ax, ax
    mov       cx, 32767
    rep       stosw          ; Clear the screen

    mov       dx, $03D4
    mov       ax, $14
    out       dx, ax         ; Disable DWORD mode

    mov       ax, $0E317     ; Enable byte mode
    out       dx, ax
    out       dx, ax
    mov       ax, $0409      ; Set cell height
    out       dx, ax

    mov       si, FirePal    ; DS:SI points to palette data
    mov       dx, $03C8      ; Palette write register
    mov       al, 0          ; Start at colour index 0
    out       dx, al
    inc       dx
    mov       cx, 768

.PALLOOP:

    outsb                    ; Output colour data
    dec       cx
    jnz       .PALLOOP

    ret

;END FIRE_INIT


FIRE_RANDOM:                 ; Generates psuedo-random numbers

    mov  ax, [FireSeed]      ; Use current seed to generate new number
    mov  dx, $8405
    mul  dx                  ; Multiply AX by DX, result in DX:AX
    inc  ax
    mov  [FireSeed], ax      ; Store seed
    ret                      ; Return to calling address

;END FIRE_RANDOM


START:                                 ; Main code section

    call      FIRE_INIT                ; Initialise screen

    mov       WORD [FireSeed], $1234   ; Initialse random number seed

    mov       si, FireScreen           ; Clear virtual screen buffer
    mov       cx, $2300
    xor       ax, ax
    rep       stosb

.MAINLOOP:

    mov       si, FireScreen
    add       si, $2300
    sub       si, 80
    mov       cx, 80
    xor       dx, dx

.NEWLINE:

    call      FIRE_RANDOM
    mov       [ds:si], dl
    inc       si
    dec       cx
    jnz       .NEWLINE

    mov       cx, $2300
    sub       cx, 80
    mov       si, FireScreen
    add       si, 80

.FIRELOOP:

    xor       ax, ax
    mov       al, [ds:si]
    add       al, [ds:si + 1]
    adc       ah, 0
    add       al, [ds:si - 1]
    adc       ah, 0
    add       al, [ds:si + 80]
    adc       ah, 0
    shr       ax, 2
    jz        .ZERO
    dec       ax

.ZERO:

    mov       [ds:si - 80], al
    inc       si
    dec       cx
    jnz       .FIRELOOP

    mov       dx, $03DA

.L1:
    in        al, dx
    and       al, $08
    jnz       .L1

.L2:
    in        al, dx
    and       al, $08
    jz        .L2

    mov       cx, $2300
    shr       cx, 1
    mov       si, FireScreen
    xor       di, di
    rep       movsw

    mov       ah, $01
    int       $16        ; Check for keypress
    jz        .MAINLOOP

    mov       ah, $00
    int       $16        ; Clear input buffer

    mov       ax, $0003  ; Set 80x25 text mode
    int       $10

    mov       dx, EndMessage
    mov       ah, $09
    int       $21        ; Display end message using DOS function call

    mov       ax, $4C00  ; This function exits the program
    int       $21        ; and returns control to DOS.

;END START
