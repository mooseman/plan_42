/*****************************************************************************
SYMBOL TABLE LOOKUP FOR DJGPP COFF AND WIN32 PE COFF

EXPORTS:
int get_coff_sym(unsigned char *image, unsigned i,
		unsigned *sym_val, unsigned *sect_num);
*****************************************************************************/
#include <stdio.h> /* printf() */
#include "defs.h"

/* IMPORTS
from RUN.C */
int lookup_external_symbol(char *sym_name, unsigned *adr, unsigned uscore);
/*****************************************************************************
*****************************************************************************/
int get_coff_sym(unsigned char *image, unsigned i,
		unsigned *sym_val, unsigned *sect_num)
{
	char *sym_name, *strtab;
	coff_file_t *file;
	coff_sym_t *sym;
//	unsigned len;
	int err;

	file = (coff_file_t *)image;
/* number of symbol table entries */
	if(i >= file->num_syms)
	{
		printf("index into symbol table (%u) is too large "
			"(%lu max)\n", i, file->num_syms);
		return -1;
	}
/* point to symtab entry, get name */
	sym = (coff_sym_t *)(image + file->symtab_offset) + i;
	sym_name = sym->x.name;
//	len = 8;
	if(sym->x.x.zero == 0)
	{
		strtab = (char *)image + file->symtab_offset +
			file->num_syms * sizeof(coff_sym_t);
		sym_name = strtab + sym->x.x.strtab_index;
//		len = strlen(sym_name);
	}
/* get section and check it */
	if(sym->sect_num > file->num_sects)
	{
		printf("symbol '%-8s' has bad section %d (max %u)\n",
			sym_name, sym->sect_num, file->num_sects);
		return -1;
	}
	*sect_num = sym->sect_num;
/* external symbol */
	if(*sect_num == 0)
	{
		err = lookup_external_symbol(sym_name, sym_val, 1);
		if(err != 0)
			return err;
	}
/* internal symbol */
	else
		*sym_val = sym->val;
	return 0;
}
