%macro malign 1
    times (($$-$) % (%1)) db 0  ;even %1 byte
%endmacro

%macro calle 1
	;Call if equal
	jne	%%next
	call	%1
	jmp	done
	%%next:
%endmacro

    ;Table register - GDT or LDT depending on which has been selected
	%define	d_table		ss:esp
	%define	d_table_lim	ss:esp			;2 bytes
	%define	d_table_base	ss:esp + 2		;4 bytes

	
