;
; Exec/Alloc/RAM.inc
;

; This module provides RAM-related functions, such as Allocating and Zeroing
; blocks of RAM. It could use the RAM Map discovered from the BIOS during Boot,
; but that's been left as an exercise for the reader!
;
; Instead, the allocator merely keeps track of the last allocated address as a
; "high water mark", returning zero if that mark becomes greater than what's
; available. Of course, this naïve implementation merely keeps giving more and
; more - with enough intelligence to skip from RAMTop to the 1 MiB mark.

;...............................................................................
; This function allocates a block of RAM from the available pool.
; Note that it doesn't allocate a Descriptor for it, since it doesn't know
; what type it'll be, nor which Table to put it in.
;
; Also note that this implementation is NOT thread-safe. It assumes that only
; one Task will ever be accessing it at the same time. To work around that,
; either some kind of lock will be required, or atomic operations such as used
; in the 80486 (such as XADD) should be used.
;
; Input:  DS  = Data
;         ECX = Size requested
; Output: EAX = Address of allocated RAM, or zero if insufficient.
;         ECX preserved
Exec.Alloc.RAM:
                MOV             EAX,[Data.RAM]    ; Get current high-water mark
.Retry:
                LEA             EDX,[EAX+ECX+0Fh] ; Get Top, rounded to paragraph
                AND             EDX,~0Fh          ; Well, it is now!
                CMP             EDX,[Data.RAMTop] ; Too far?
                JBE             .Save             ; Not yet...

                MOV             EAX,0010_0000h    ; Skip to 1 MiB
                MOV    DWORD    [Data.RAMTop],-1  ; And pretend the rest is ours!
                JMP             .Retry            ; And try again
.Save:
                MOV             [Data.RAM],EDX    ; Save new high-water mark
.End:
                RET

;...............................................................................
; This function zeroes out the block of passed-in RAM.
; Input:  ES  = Segment to zero
;         ECX = Size to zero
;         EDI = Where to zero
; Output: ES and ECX preserved.
;         EAX and EDI? Not so much!
Exec.Alloc.RAM.Zero:
                PUSH            ECX             ; As promised...

                CLD                             ; Work forwards

                XOR             EAX,EAX         ; Store zeroes
                SHR             ECX,2           ; Do DWORDs initially
                REP             STOSD           ; Zero out whole DWORDs at a time

                MOV             ECX,[ESP]       ; Recover saved value
                AND             ECX,03h         ; Mask off DWORDs already done
                REP             STOSB           ; Make up any last bytes

                POP             ECX
                RET
