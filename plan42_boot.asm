;*********************************************
;	plan42_boot.asm
;	Bootloader for Plan 42 
;       
;       Acknowledgement -  Very many thanks to the 
;       people who wrote the Operating Systems 
;       Development series of tutorials at 
;       brokenthron.com.    
;*********************************************
 
org    0x7c00				; We are loaded by BIOS at 0x7C00
                                                ; org is the origin location.  
 
bits	16					; We are in 16 bit Real Mode
 
Start:
	cli					; Clear all Interrupts
	hlt					; halt the system
	
times 510 - ($-$$) db 0				; We have to be 512 bytes. Clear the rest of the bytes with 0
 
dw 0xAA55					; Boot Signature  
