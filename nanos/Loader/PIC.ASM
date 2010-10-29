;%macro pause 0
;    jmp short $+2
;    jmp short $+2
;%endmacro

              ;bit3: 0:edge(PS/2) 1:level(80x86) triggered mode (system)
    ICW1 equ 15h   ;must contain: 0001????b (sign for ICW1)
              ;vector address (?????xxx xxx=none)
    ICW2  equ IRQBase  ;ICW2 = 10000xxx (IRQ 0-7  INT 80h - 87h)
    ICW4  equ 01h  ;ICW4 = 1
    OCW1m equ 11111100b ;OCW1 = 11111111b (IMR(Mask)IRQ 0-7)  0:enable 1:disable
    OCW1s equ 11111111b ;OCW1 = 11111111b (IMR(Mask)IRQ 8-15)  0:enable 1:disable

; reprogram 8259-compatible interrupt controllers to use INT 20h through
; INT 2Fh for the 16 hardware interrupts. Code from Josh McDonald's OS/2000
; <http://www.effect.net.au/os-dev/> and from Linux.
    cli
	mov al,ICW1		; put both 8259s in init mode
	out 0x20,al
	out 0xA0,al
	mov al,ICW2		; IRQ0-IRQ7 -> interrupts 0x20-0x27
	out 0x21,al
	add al,8
	out 0xA1,al		; IRQ8-IRQ15 -> interrupts 0x28-0x2F
	mov al,4
	out 0x21,al
	mov al,2
	out 0xA1,al
	mov al,ICW4
	out 0x21,al
	out 0xA1,al
; enable IRQs at these chips [ints still disabled at CPU]
	mov al,OCW1m		; IRQ0 [timer]
	out 0x21,al
	mov al,OCW1s		; none
	out 0xA1,al
