; gdt.inc   symbols and macros for building descriptors
; Version 2.3, Mar 16, 1998
; Sample code
; by John S. Fine  johnfine@erols.com
; I do not place any restrictions on your use of this source code
; I do not provide any warranty of the correctness of this source code
;_____________________________________________________________________________
;
;  This gdt.inc uses features that I added to NASM.
;  As I write this, the only version of NASM with those features is available
;  on my web page.
;  http://www.erols.com/johnfine/#nasmj
;
;  This gdt.inc will also assemble correctly with standard NASM version 0.97
;  (It tests for a feature I added to NASM and does things a different way
;   if that feature is missing).  However, using standard NASM 0.97 every
;  use of the desc macro inside the gdt must define a selector.  With my
;  NASM, selectors may be defined or skipped on each use of desc.
;_____________________________________________________________________________
;
; The desc macro pieces together a segment descriptor.
;
; desc  offset, selector, control   ;For gate descriptors
; desc  base, limit, control	    ;For all other descriptors
;
;  base    is the full 32 bit base address of the segment
;  limit   is one less than the segment length in 1 or 4K byte units
;  control the sum of all the "D_" equates which apply (for call gates, you
;          also add the "parameter dword count" to flags).
;
;  Descriptors are built in a simplified format at assembly time, and a
;  special symbol is defined to tell JLOC to convert to correct format.
;  See notes below.
;
;  There is special handling for any label that occurs on the line with
;  the desc macro, for example:
;
;  flat_code:  desc  0, 0xFFFFF, D_CODE + D_READ + D_BIG + D_BIG_LIM
;
;  This defines a standard flat code segment with a base of zero.  The
;  symbol flat_code is defined as the selector for this descriptor, not
;  the address of the descriptor.  Thus you could do:
;
;  jmp  flat_code:entry_point
;
;  or define an IDT entry point as:
;
;  desc  service_GPF, flat_code, D_TRAP
;
;  The support for defining the selectors must be set up by using something
;  like the start_gdt macro.
;
;  The start_gdt macro also fills in the null descriptor with a special
;  descriptor that can be used by the lgdt instruction.
;
;  The end_gdt macro defines the limit value needed by start_gdt and also
;  releases the NASM context used for defining selectors
;_____________________________________________________________________________

;Each descriptor should have exactly one of next 8 codes to define the type of
;descriptor
D_LDT		EQU	 200h	;LDT segment
D_TASK		EQU	 500h	;Task gate
D_TSS		EQU	 900h	;TSS
D_CALL		EQU	0C00h	;386 call gate
D_INT		EQU	0E00h	;386 interrupt gate
D_TRAP		EQU	0F00h	;386 trap gate
D_DATA		EQU	1000h	;Data segment
D_CODE		EQU	1800h	;Code segment

;Descriptors may include the following as appropriate:
D_DPL3		EQU	6000h	;DPL3 or mask for DPL
D_DPL2		EQU	4000h
D_DPL1		EQU	2000h
D_PRESENT	EQU	8000h	;Present
D_NOT_PRESENT	EQU	8000h	;Not Present
				;Note, the PRESENT bit is set by default
				;Include NOT_PRESENT to turn it off
				;Do not specify D_PRESENT

;Segment descriptors (not gates) may include:
D_ACC		EQU	 100h	;Accessed (Data or Code)

D_WRITE		EQU	 200h	;Writable (Data segments only)
D_READ		EQU	 200h	;Readable (Code segments only)
D_BUSY		EQU	 200h	;Busy (TSS only)

D_EXDOWN	EQU	 400h	;Expand down (Data segments only)
D_CONFORM	EQU	 400h	;Conforming (Code segments only)

D_BIG		EQU	  40h	;Default to 32 bit mode (USE32)
D_BIG_LIM	EQU	  80h	;Limit is in 4K units

%macro test_for_my_00_feature 0
%ifnnum	%00
%define my_00_feature
%endif
%endm
test_for_my_00_feature

;  NASM version 0.97 has some bugs in putting %macro definitions inside %if
;  constructs.  I sidestep that problem by putting the %macro definitions in
;  %include files.
;_____________________________________________________________________________

%ifdef my_00_feature

%include "gdt_new.inc"

%else

%include "gdt_old.inc"

%endif
;-----------------------------------------------------------------------------
;  Note:
;
;  In true descriptors the offset, base and limit fields are each broken into
;  two parts.  It is often useful to define descriptors for which these fields
;  (especially offset) are not known until link time.  Since the assembler
;  doesn't know the final value, it can't split the fields correctly.
;
;  To work around this problem, I have the desc macro store the parts of each
;  descriptor as contiguous fields, rather than in the correct Intel sequence.
;  The assembler can use standard "fixup" requests to indicate any values that
;  are not known at assembly time.  You must link with JLOC.  After applying
;  the fixups, JLOC will rearrange the descriptor into the correct sequence
;
;  There are two basic formats for descriptors, one for gates, one for
;  everything else.
;
;  A gate is identified by JLOC as any descriptor whose simplified format has
;  bit 26 set and bit 28 clear.
;
;  For a gate, the following rearrangement occurs:
;
;  subField             Simplified location   Final location
;  ------------------   -------------------   --------------
;  Selector[0..15]            0..15               16..31
;  Minor control bits        16..23               32..39
;  Major control bits        24..31               40..47
;  Offset[0..15]             32..47                0..15
;  Offset[16..31]            48..63               48..63
;
;  For non-gates the following rearrangement occurs:
;
;  subField             Simplified location   Final location
;  ------------------   -------------------   --------------
;  Limit[0..15]               0..15                0..15
;  Limit[16..19]             16..19               48..51
;  Minor control bits        20..23               52..57
;  Major control bits        24..31               40..47
;  Base[0..23]               32..55               16..39
;  Base[24..31]              56..63               56..63
;
;  The last parameter to the desc macro contains all the control bits
;  combined.  It is generated by adding together the appropriate
;  D_ constants.  For all descriptors, it has the major control bits in D_
;  bits 8 to 15.  The minor control bits are in either D_ bits 0 to 7 or bits
;  4 to 7 depending on the type of descriptor.
;_____________________________________________________________________________

