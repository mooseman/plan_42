; 8259.inc  Definitions for the 8259 interrupt controller
; Version 1.1, Nov 29, 1997
; Sample code
; by John S. Fine  johnfine@erols.com
; I do not place any restrictions on your use of this source code
; I do not provide any warranty of the correctness of this source code
;_____________________________________________________________________________

M_PIC	EQU	0x20		;I/O for master PIC
M_IMR	EQU	0x21		;I/O for master IMR
S_PIC	EQU	0xA0		;I/O for slave PIC
S_IMR	EQU	0xA1		;I/O for slace IMR

EOI	EQU	0x20		;EOI command

ICW1	EQU	0x11		;Cascade, Edge triggered
				;ICW2 is vector
				;ICW3 is slave bitmap or number
ICW4	EQU	0x01		;8088 mode

M_VEC	EQU	0x68		;Vector for master
S_VEC	EQU	0x70		;Vector for slave

OCW3_IRR EQU	0x0A		;Read IRR
OCW3_ISR EQU	0x0B		;Read ISR
