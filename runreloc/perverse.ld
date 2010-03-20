/* unusual (!) ordering of sections to stress-test relocation code */
SECTIONS
{
    .bss :
    {
	*(.bss)
	*(COMMON)
	. = ALIGN(4096);
    }
    .dll :
    {
	*(.dll)
	. = ALIGN(4096);
    }
    .data :
    {
	*(.data)
	. = ALIGN(4096);
    }
/* if .text is not the first section in the file,
DJGPP ld adds an a.out header to .o file...strange... */
    .text :
    {
	*(.text)
	. = ALIGN(4096);
    }
}
