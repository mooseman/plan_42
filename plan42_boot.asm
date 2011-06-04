
;   Plan 42 boot loader.  
;   Acknowledgements - Very many thanks to Daniel Faulkner on the 
;   osdever forum, and to Mike (of brokenthorn.com). 
;   The bootloader of this OS has code from both Daniel and from 
;   Mike's **outstanding** OS development tutorial series. 
;   Mike has very generously released the code in those tutorials to 
;   the public domain. This OS would not have been possible without 
;   the use of that excellent resource!  

;   This code is released to the public domain.    

;   Compile this code using this command - 
;   nasm -f bin plan42_boot.asm -o plan42_boot.bin

[BITS 16]      ; 16 bit code generation
[ORG 0x7C00]   ; Origin location. This MUST be 0x7C00 for a 
               ; boot-loader. See OSDev article 3 for details - 
               ; http://www.brokenthorn.com/Resources/OSDev3.html  

; Jump over the parameter block to the main program  
start: jmp main 


;********************************************************
; BIOS parameter block. This describes the layout of the 
; filesystem. 
;******************************************************** 
TIMES 0Bh-$+start DB 0

bpbBytesPerSector:  	DW 512
bpbSectorsPerCluster: 	DB 1
bpbReservedSectors: 	DW 1
bpbNumberOfFATs: 	    DB 2
bpbRootEntries: 	    DW 224
bpbTotalSectors: 	    DW 2880
bpbMedia: 	            DB 0xF0
bpbSectorsPerFAT: 	    DW 9
bpbSectorsPerTrack: 	DW 18
bpbHeadsPerCylinder: 	DW 2
bpbHiddenSectors: 	    DD 0
bpbTotalSectorsBig:     DD 0
bsDriveNumber: 	        DB 0
bsUnused: 	            DB 0
bsExtBootSignature: 	DB 0x29
bsSerialNumber:	        DD 0xa0a1a2a3
bsVolumeLabel: 	        DB "PLAN 42    "
bsFileSystem: 	        DB "FAT12   " 



; Main program
main:      ; Label for the start of the main program

mov ax,0x0000   ; Setup the Data Segment register
      ; Location of data is DS:Offset
mov ds,ax   ; This can not be loaded directly it has to be in two steps.
      ; 'mov ds, 0x0000' will NOT work due to limitations on the CPU

mov si, str1  ; Load the string into position for the procedure.
call PutStr   ; Call/start the procedure
mov si, str2  ; Load another string
call PutStr     ;  Print string   

jmp $      ; Never ending loop

; Procedures
PutStr:      ; Procedure label/start
; Set up the registers for the interrupt call
mov ah,0x0E   ; The function to display a chacter (teletype)
mov bh,0x00   ; Page number
mov bl,0x07   ; Normal text attribute

.nextchar   ; Internal label (needed to loop round for the next character)
lodsb      ; I think of this as LOaD String Block
      ; (Not sure if thats the real meaning though)
      ; Loads [SI] into AL and increases SI by one
; Check for end of string '0'
or al,al   ; Sets the zero flag if al = 0
      ; (OR outputs 0's where there is a zero bit in the register)
jz .return   ; If the zero flag has been set go to the end of the procedure.
      ; Zero flag gets set when an instruction returns 0 as the answer.
int 0x10   ; Run the BIOS video interrupt
jmp .nextchar   ; Loop back round tothe top
.return      ; Label at the end to jump to when complete
ret      ; Return to main program

; Data

str1 db 'Open the pod-bay door, HAL...',13,10,0
str2 db "I'm sorry.... I can't do that, Dave. ",13,10,0   

; End Matter
times 510-($-$$) db 0   ; Fill the rest with zeros
dw 0xAA55      ; Boot loader signature 

 
cli				; clear the interrupts  
hlt             ; halt the system 

