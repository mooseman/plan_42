PHDRS
{
  text PT_LOAD FLAGS(1) ;
  data PT_LOAD FLAGS(2 + 4) ;
}

ENTRY(start)
OUTPUT_FORMAT("elf32-i386")

SECTIONS
{
	. = SIZEOF_HEADERS ;

	.text 0 : {
		*(.text)
	} :text
	
	.data 0 : {
		*(.data)
	} :data
	
	.bss (NOLOAD) : {
		*(.bss COMMON)
	} :data

	/DISCARD/ : {
		 *(*)
	}
}
