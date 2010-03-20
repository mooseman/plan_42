; event.inc   Event data structure
; Version 1.1, Nov 29, 1997
; Sample code
; by John S. Fine  johnfine@erols.com
; I do not place any restrictions on your use of this source code
; I do not provide any warranty of the correctness of this source code
;_____________________________________________________________________________

	struc	EVT
EVT_time   resd 1	;Time at which this event should next occur
EVT_heap   resd 1	;Backpointer to this event's pointer in the heap
EVT_code   resd 1	;Subroutine to execute when this event occurs
EVT_value  resd 1	;Identify this events (vs. others with the same code)
EVT_work   resd 1	;Workspace for the subroutine to use
	endstruc
