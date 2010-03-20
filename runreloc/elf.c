/*****************************************************************************
ELF RELOCATION ROUTINE

EXPORTS:
int load_elf_relocatable(unsigned char *image, unsigned *entry);
*****************************************************************************/
#include <stdlib.h> /* calloc() */
#include <string.h> /* strcmp() */
#include <stdio.h> /* printf() */
#include "defs.h"

/* IMPORTS
from RUN.C */
int lookup_external_symbol(char *sym_name, unsigned *adr, unsigned uscore);

typedef struct
{
	uint32_t magic		__PACKED__;
	uint8_t bitness		__PACKED__;
	uint8_t endian		__PACKED__;
	uint8_t elf_ver_1	__PACKED__;
	uint8_t res[9]		__PACKED__;
	uint16_t file_type	__PACKED__;
	uint16_t machine	__PACKED__;
	uint32_t elf_ver_2	__PACKED__;
	uint32_t entry_pt	__PACKED__;
	uint32_t phtab_offset	__PACKED__;
	uint32_t shtab_offset	__PACKED__;
	uint32_t flags		__PACKED__;
	uint16_t file_hdr_size	__PACKED__;
	uint16_t phtab_ent_size	__PACKED__;
	uint16_t num_phtab_ents	__PACKED__;
	uint16_t shtab_ent_size	__PACKED__;
	uint16_t num_sects	__PACKED__;
	uint16_t shstrtab_index	__PACKED__;
} elf_file_t;

typedef struct
{
	uint32_t sect_name	__PACKED__;
	uint32_t type		__PACKED__;
	uint32_t flags		__PACKED__;
	uint32_t virt_adr	__PACKED__;
	uint32_t offset		__PACKED__;
	uint32_t size		__PACKED__;
	uint32_t link		__PACKED__;
	uint32_t info		__PACKED__;
	uint32_t align		__PACKED__;
	uint32_t ent_size	__PACKED__;
} elf_sect_t;

typedef struct
{
	uint32_t adr		__PACKED__;
	uint8_t type		__PACKED__;
	uint32_t symtab_index : 24 __PACKED__;
	uint32_t addend		__PACKED__;
} elf_reloc_t;

typedef struct
{
	uint32_t name		__PACKED__;
	uint32_t value		__PACKED__;
	uint32_t size		__PACKED__;
	unsigned type : 4	__PACKED__;
	unsigned binding : 4	__PACKED__;
	uint8_t zero		__PACKED__;
	uint16_t section	__PACKED__;
} elf_sym_t;
/*****************************************************************************
get value of symbol #i
*****************************************************************************/
static int get_elf_sym(unsigned char *image, unsigned i,
		unsigned *sym_val, unsigned symtab_sect)
{
	elf_file_t *file;
	elf_sect_t *sect;
	elf_sym_t *sym;
	char *sym_name;
	unsigned adr;
	int err;

/* point to symbol table */
	file = (elf_file_t *)image;
	if(symtab_sect >= file->num_sects)
	{
		printf("bad symbol table section number %d (max %u)\n",
			symtab_sect, file->num_sects - 1);
		return -1;
	}
	sect = (elf_sect_t *)(image + file->shtab_offset +
		file->shtab_ent_size * symtab_sect);
/* get symbol */
	if(i >= sect->size)
	{
		printf("offset into symbol table (%u) exceeds symbol "
			"table size (%lu)\n", i, sect->size);
		return -1;
	}
	sym = (elf_sym_t *)(image + sect->offset) + i;
/* external symbol */
	if(sym->section == 0)
	{
/* point to string table for this symbol table */
		sect = (elf_sect_t *)(image + file->shtab_offset +
			file->shtab_ent_size * sect->link);
/* get symbol name */
		sym_name = (char *)image + sect->offset + sym->name;
/* ELF binutils for DJGPP: leading underscore
		err = lookup_external_symbol(sym_name, sym_val, 1); */
/* Linux: no leading underscore */
		err = lookup_external_symbol(sym_name, sym_val, 0);
		if(err != 0)
			return err;
	}
/* internal symbol */
	else
	{
		sect = (elf_sect_t *)(image + file->shtab_offset +
			file->shtab_ent_size * sym->section);
		adr = (unsigned)image + sect->offset;
		*sym_val = sym->value + adr;
	}
	return 0;
}
/*****************************************************************************
*****************************************************************************/
static int do_elf_reloc(unsigned char *image, elf_reloc_t *reloc,
		elf_sect_t *sect)
{
	unsigned t_adr, sym_val;
	elf_sect_t *t_sect;
	elf_file_t *file;
	uint32_t *where;
	int err;

/* get address of target section */
	file = (elf_file_t *)image;
	t_sect = (elf_sect_t *)(image + file->shtab_offset +
		file->shtab_ent_size * sect->info);
	t_adr = (unsigned)image + t_sect->offset;
/* point to relocation */
	where = (uint32_t *)(t_adr + reloc->adr);
/* get symbol */
	err = get_elf_sym(image, reloc->symtab_index, &sym_val, sect->link);
	if(err != 0)
		return err;
	switch(reloc->type)
	{
/* absolute reference
Both ELF.H and objdump call this "R_386_32" */
	case 1: /* S + A */
		*where = sym_val + *where;
		break;
/* EIP-relative reference
Both ELF.H and objdump call this "R_386_PC32" */
	case 2: /* S + A - P */
		*where = sym_val + *where - (unsigned)where;
		break;
	default:
		printf("unknown/unsupported relocation type %u "
			"(must be 1 or 2)\n", reloc->type);
		return -1;
	}
	return 0;
}
/*****************************************************************************
*****************************************************************************/
int load_elf_relocatable(unsigned char *image, unsigned *entry)
{
	unsigned s, r, reloc_size;
	unsigned char *bss;
	elf_reloc_t *reloc;
	elf_sect_t *sect;
	elf_file_t *file;
	int err;

/* validate */
	file = (elf_file_t *)image;
	if(file->magic != 0x464C457FL) /* "ELF" */
	{
		printf("File is not relocatable ELF; has bad magic value "
			"0x%lX (should be 0x464C457F)\n", file->magic);
		return +1;
	}
	if(file->bitness != 1)
	{
		printf("File is 64-bit ELF, not 32-bit\n");
		return -1;
	}
	if(file->endian != 1)
	{
		printf("File is big endian ELF, not little\n");
		return -1;
	}
	if(file->elf_ver_1 != 1)
	{
		printf("File has bad ELF version %u\n", file->elf_ver_1);
		return -1;
	}
	if(file->file_type != 1)
	{
		printf("File is not relocatable ELF (could be "
			"executable, DLL, or core file)\n");
		return -1;
	}
	if(file->machine != 3)
	{
		printf("File is not i386 ELF\n");
		return -1;
	}
	if(file->elf_ver_2 != 1)
	{
		printf("File has bad ELF version %lu\n", file->elf_ver_2);
		return -1;
	}
/* find the BSS and allocate memory for it
This must be done BEFORE doing any relocations */
	for(s = 0; s < file->num_sects; s++)
	{
		sect = (elf_sect_t *)(image + file->shtab_offset +
				file->shtab_ent_size * s);
		if(sect->type != 8)	/* NOBITS */
			continue;
		r = sect->size;
		bss = calloc(1, r);
		if(bss == NULL)
		{
			printf("Can't allocate %u bytes for BSS\n",
				r);
			return -1;
		}
		sect->offset = bss - image;
		break;
	}
/* for each section... */
	for(s = 0; s < file->num_sects; s++)
	{
		sect = (elf_sect_t *)(image + file->shtab_offset +
				file->shtab_ent_size * s);
/* is it a relocation section?
xxx - we don't handle the extra addend for RELA relocations */
		if(sect->type == 4)	/* RELA */
			reloc_size = 12;
		else if(sect->type == 9)/* REL */
			reloc_size = 8;
		else
			continue;
/* for each relocation... */
		for(r = 0; r < sect->size / reloc_size; r++)
		{
			reloc = (elf_reloc_t *)(image + sect->offset +
				reloc_size * r);
			err = do_elf_reloc(image, reloc, sect);
			if(err != 0)
				return err;
		}
	}
/* find start of .text and make it the entry point */
	(*entry) = 0;
	for(s = 0; s < file->num_sects; s++)
	{
		sect = (elf_sect_t *)(image + file->shtab_offset +
				file->shtab_ent_size * s);
		if((sect->flags & 0x0004) == 0)
			continue;
		(*entry) = (unsigned)image + sect->offset;
		break;
	}
	if((*entry) == 0)
	{
		printf("Can't find section .text, so entry point is unknown\n");
		return -1;
	}
	return 0;
}
