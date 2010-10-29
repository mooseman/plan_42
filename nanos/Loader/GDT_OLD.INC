; gdt_old.inc   symbols and macros for building descriptors
; Version 2.3, Mar 16, 1998
; Sample code
; by John S. Fine  johnfine@erols.com
; I do not place any restrictions on your use of this source code
; I do not provide any warranty of the correctness of this source code
;_____________________________________________________________________________
;
;  Use gdt.inc to include this file.
;  This section was split out of gdt.inc because of a bug in NASM version
;  0.97 handling of macro definitions within an %if.
;
;  This varient of the macro definitions is used if my %00 feature is not
;  present in NASM
;_____________________________________________________________________________

%define fix_nasm_prepend_bug
%define desc_make_selector

%macro start_gdt 0
fix_nasm_prepend_bug
%push table
%$startoftable:
	dw	%$limitoftable
	dd	%$startoftable
	dw	0
%define desc_make_selector equ $-%$startoftable
%endmacro

%macro end_gdt 0
%$limitoftable	equ	$-%$startoftable-1
%define desc_make_selector
%pop
%endmacro

%macro desc 3
desc_make_selector
	GLOBAL	?fixD%%jloc
?fixD%%jloc:
	dd	(%2) + ( ( (%3) ^ D_PRESENT ) << 16 ) , (%1)
%endmacro



