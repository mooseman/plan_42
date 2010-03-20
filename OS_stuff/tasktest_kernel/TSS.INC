; tss.inc   structure for building task segments
; Version 1.0 March 5, 1998
; Sample code
; by John S. Fine  johnfine@erols.com
; I do not place any restrictions on your use of this source code
; I do not provide any warranty of the correctness of this source code
;_____________________________________________________________________________

	struc	tss
.back	resw	2
.esp0	resd	1
.ss0	resw	2
.esp1	resd	1
.ss1	resw	2
.esp2	resd	1
.ss2	resw	2
.cr3	resd	1
.eip	resd	1
.eflags	resd	1
.eax	resd	1
.ecx	resd	1
.edx	resd	1
.ebx	resd	1
.esp	resd	1
.ebp	resd	1
.esi	resd	1
.edi	resd	1
.es	resw	2
.cs	resw	2
.ss	resw	2
.ds	resw	2
.fs	resw	2
.gs	resw	2
.ldt	resw	2
.trap	resw	1
.io	resw	1
	endstruc

