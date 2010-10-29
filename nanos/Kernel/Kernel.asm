;Kernel for windows
;Version 0.01

;Start of kernel services
    nanos_version  equ  004h

%include 'macro.asm'
%include '../const.asm'
%include '../mem.asm'
%include '../struc.asm'
%include 'err_code.asm'


SEGMENT .text

kernel_start:
;Startup code
sti

%include 'rs232.asm'

PrintByte '-'
PrintByte 'K'
PrintByte 'e'
PrintByte 'r'
PrintByte 'n'
PrintByte 'e'
PrintByte 'l'
PrintByte '-'

;jump to idle task
jmp	Multitasking.idle


;Kernel services
global	service
%include 'service.asm'


;=====================
Multitasking:
;Multitasking timer interrupt
global   Multitasking.interrupt
global   Multitasking.device_not_available
global   Multitasking.tss


    malign 4

.interrupt:
%include 'Mult_Int.asm'

;TSS for Idle Task

    malign 4

.tss:
%include 'idle_tss.asm'


;Idle Code

    malign 4

.idle:
%include 'idle.asm'


;=====================
;Interrupt managers
;
;Exception interrupt 0-1F
;IRQ-Interrupt		 20-2F	(20 = timer, multitasking)
    malign 4
Interrupt:
%include 'Interrupt.asm'
