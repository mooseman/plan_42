; pri_heap.asm  Manage priority heap
; Version 1.1, Nov 29, 1997
; Sample code
; by John S. Fine  johnfine@erols.com
; I do not place any restrictions on your use of this source code
; I do not provide any warranty of the correctness of this source code
;_____________________________________________________________________________
;
;  Manage a priority heap:
;
;  These routines assume that at most one event is out of place within the
;  heap.
;
;  sift is used if that event is potentially too early in the heap.
;  anti_sift is used if that event is potentially too late in the heap.
;  sift_heap is an entry to sift, used for the first event in the heap.
;  anti_sift_new is an entry into anti_sift, used for adding events.
;
;  Common uses:
;
;  When an event is executed, its next execution time is recomputed.  Call
;  sift_heap to correct its location in the heap.
;
;  When an event is new, or a suspended event is resumed, and must be added
;  to the heap, call anti_sift_new.
;
;  When an event must be removed from the heap, you take the last event off
;  the end of the heap as a replacement for the removed event.  Call sift
;  to adjust the position.
;_____________________________________________________________________________
;
; Warning: anti_sift does tricks with pointers into pri_heap.  As presently
; coded, it relies on the fact that pri_heap is in the top half of the
; address space.  It also relies on the align=4.
;_____________________________________________________________________________

MAX_EVENTS  EQU  1024		;Size of priority heap

	GLOBAL	sift
	GLOBAL	sift_heap
	GLOBAL	anti_sift
	GLOBAL	anti_sift_new
	GLOBAL	pri_heap
	GLOBAL	heap_last

	%include "event.inc"

	extern	heap_adj	;heap_adj equ (4-pri_heap), but an assembler
				;can't negate a relative quantity, so I
				;have JLOC compute it.

SEGMENT CODE USE32

sift_heap:
;
;Purpose:
;	Adjust the heap for an event whose time might be later than
;	is appropriate for its position in the heap.
;Input:
;	none
;Output:
;	none
;Clobbers:
;	eax, ebx, ecx, edx, esi, edi
;_____________________________________________________________________________
	mov	ebx, pri_heap

sift:
;
;Purpose:
;	Adjust the heap for an event whose time might be later than
;	is appropriate for its position in the heap.
;Input:
;	ebx = starting position within heap
;Output:
;	none
;Clobbers:
;	eax, ebx, ecx, edx, esi, edi
;_____________________________________________________________________________

	mov	ecx, [ebx]		;Event which must be checked
.10:	lea	eax, [ebx*2+heap_adj]	;First branch
	cmp	eax, [heap_last]	;Compare to last address
	ja	.50			;Beyond last
	mov	edi, [eax]		;Get event at first branch
	mov	edx, [EVT_time+edi]	;Get execution time of first branch
	je	.30			;Exactly last
	mov	esi, [eax+4]		;Get event at second branch
	cmp	edx, [EVT_time+esi]	;Select the earlier one
	js	.30			;First branch is earlier
	add	eax, byte 4		;Second branch is earlier
	mov	edx, [EVT_time+esi]
	mov	edi, esi
.30:	cmp	edx, [EVT_time+ecx]	;Compare to new time
	jns	.50			;New time is OK (earlier or equal)
	mov	[ebx], edi		;Update heap
	mov	[EVT_heap+edi], ebx	;Update backlink
	mov	ebx, eax		;Next location
	jmp short .10
.50:	mov	[ebx], ecx		;Update heap
	mov	[EVT_heap+ecx], ebx	;Update backlink
	ret
	
anti_sift_new:
;
;Purpose:
;	Add a new event to the heap
;Input:
;	ebx = new event
;Output:
;	none
;Clobbers:
;	eax, ecx, edx, esi
;_____________________________________________________________________________

	add dword [heap_last], byte 4	;Grow priority heap
	mov	eax, [heap_last]	;New last address

anti_sift:
;
;Purpose:
;	Adjust the heap for an event whose time might be earlier than
;	is appropriate for its position in the heap.
;Input:
;	eax = position within the heap
;	ebx = event
;Output:
;	none
;Clobbers:
;	eax, ecx, edx, esi
;_____________________________________________________________________________

	mov	edx, [EVT_time+ebx]	;New time
.10:	cmp	eax, pri_heap		;Back to start?
	je	.70			;Yes
	lea	ecx, [eax+pri_heap-3]	;Up one level
	ror	ecx, 1			;  (warning: Assumes high address)
	and	ecx, byte ~2		;  (change -3 to -4 for low half)
	mov	esi, [ecx]		;Get that event
	cmp	edx, [EVT_time+esi]	;New time OK?
	jns	.70			;Yes
	mov	[eax], esi		;No, update heap
	mov	[EVT_heap+esi], eax	;Update backlink
	mov	eax, ecx		;Next location
	jmp short .10
.70:	mov	[eax], ebx		;Update heap
	mov	[EVT_heap+ebx], eax	;Update backlink
	ret

SEGMENT BSS USE32 align=4
pri_heap resd MAX_EVENTS

SEGMENT DATA USE32 align=4
heap_last  dd	pri_heap-4
