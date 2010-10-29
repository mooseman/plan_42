;============================================
;Allocation Unit
;
; Written by: Peter Hultqvist
;             peter.hultqvist@neocoder.net
;
; Description:
;	This is a memory/paging/ioports/anything allocation unit
;	
;	Allocation is used in blocks
;
;============================================


size		equ	2^32			;Size of entire area

min_bits	equ	12
max_bits	equ	32
min_size	equ	2^min_bits	;Minimum block size
max_size	equ	2^max_bits	;Maximum block size




;Structure:
;	0	4	Start
;	4	4	Size
;	8	4	Next linear pointer
;	C	4	Next same size pointer
;	10	4	Prev linear pointer
;	14	4	Prev same size pointer
