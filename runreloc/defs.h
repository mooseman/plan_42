#ifndef __MY_DEFS_H
#define	__MY_DEFS_H

#if defined(__GNUC__)
#define	__PACKED__	__attribute__((__packed__))
#else
#define	__PACKED__	/* nothing */
//#error Need compiler voodoo to pack structures.
#endif

typedef unsigned char	uint8_t;
typedef unsigned short	uint16_t;
typedef unsigned long	uint32_t;

typedef struct
{
	uint16_t magic		__PACKED__;
	uint16_t num_sects	__PACKED__;
	uint32_t time_date	__PACKED__;
	uint32_t symtab_offset	__PACKED__;
	uint32_t num_syms	__PACKED__;
	uint16_t aout_hdr_size	__PACKED__;
	uint16_t flags		__PACKED__;
/* for executable COFF file, a.out header would go here */
} coff_file_t;

typedef struct
{
	char name[8]		__PACKED__;
	uint32_t phys_adr	__PACKED__;
	uint32_t virt_adr	__PACKED__;
	uint32_t size		__PACKED__;
	uint32_t offset		__PACKED__;
	uint32_t relocs_offset	__PACKED__;
	uint32_t line_nums_offset __PACKED__;
	uint16_t num_relocs	__PACKED__;
	uint16_t num_line_nums	__PACKED__;
	uint32_t flags		__PACKED__;
} coff_sect_t;

typedef struct
{
	uint32_t adr		__PACKED__;
	uint32_t symtab_index	__PACKED__;
	uint16_t type		__PACKED__;
} coff_reloc_t;

typedef struct
{
	union
	{
		char name[8]	__PACKED__;
		struct
		{
			uint32_t zero		__PACKED__;
			uint32_t strtab_index	__PACKED__;
		} x		__PACKED__;
	} x			__PACKED__;
	uint32_t val		__PACKED__;
	uint16_t sect_num	__PACKED__;
	uint16_t type		__PACKED__;
	uint8_t sym_class	__PACKED__;
	uint8_t num_aux		__PACKED__;
} coff_sym_t;

#endif
