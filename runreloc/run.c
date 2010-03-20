/*****************************************************************************
LOAD AND RUN DJGPP COFF, WIN32 PE COFF, OR ELF RELOCATABLE FILE

EXPORTS
int lookup_external_symbol(char *sym_name, unsigned *adr, unsigned uscore);
*****************************************************************************/
#include <stdlib.h> /* NULL, malloc(), free() */
#include <string.h> /* strcmp() */
#include <setjmp.h> /* jmp_buf, setjmp(), longjmp() */
/* FILE, SEEK_..., printf(), putchar() */
#include <stdio.h> /* fopen(), fseek(), ftell(), fclose(), fread() */
#include <time.h> /* time_t, time() */
#include "defs.h"

/* IMPORTS
from CONIO.H or LINUX.C */
int getch(void);
int kbhit(void);

/* from DOS.H or LINUX.C */
void delay(unsigned milliseconds);

/* from CONSOLE.C or LINUX.C */
void sys_putch(unsigned c);

/* from DJCOFF.C */
int load_djcoff_relocatable(char *image, unsigned *entry);

/* from PECOFF.C */
int load_pecoff_relocatable(char *image, unsigned *entry);

/* from ELF.C */
int load_elf_relocatable(unsigned char *image, unsigned *entry);

typedef void (*void_fn_void_t)(void);

static jmp_buf g_oops;
#if defined(__WIN32__)
const char *g_sys_ver = "Win32 PE COFF";
#elif defined(__DJGPP__)
const char *g_sys_ver = "DJGPP COFF";
#else
const char *g_sys_ver = "ELF";
#endif
/*****************************************************************************
*****************************************************************************/
int main(int arg_c, char *arg_v[])
{
	unsigned char *image;
	unsigned long size;
	unsigned entry;
	FILE *file;
	int err;

	if(setjmp(g_oops) != 0)
		return 0;
/* usage */
	if(arg_c < 2)
	{
		printf("Specify DJGPP COFF relocatable file\n");
		return 1;
	}
/* open file */
	file = fopen(arg_v[1], "rb");
	if(file == NULL)
	{
		printf("Can't open file '%s\n", arg_v[1]);
		return 2;
	}
/* get size */
	fseek(file, 0, SEEK_END);
	size = ftell(file);
/* alloc mem */
	image = malloc(size);
	if(image == NULL)
	{
		printf("Out of memory\n");
		fclose(file);
		return 3;
	}
/* read entire file */
	fseek(file, 0, SEEK_SET);
	fread(image, 1, size, file);
	fclose(file);
/* try loading as ELF */
	err = load_elf_relocatable(image, &entry);
	if(err == 0)
		goto OK;
	if(err < 0)
		goto END;
/* try loading as COFF */
#if defined(__WIN32__)
	err = load_pecoff_relocatable(image, &entry);
#else
	err = load_djcoff_relocatable(image, &entry);
#endif
	if(err == 0)
		goto OK;
	if(err < 0)
		goto END;
OK:
	printf("Esc quits, any other key runs loaded program\n");
	if(getch() != 27)
		((void_fn_void_t)entry)();
END:
	free(image);
	return 0;
}
/*============================================================================
'KERNEL'
============================================================================*/
/*****************************************************************************
*****************************************************************************/
static void sys_write(unsigned char *buf, unsigned len)
{
	for(; len != 0; len--)
	{
		sys_putch(*buf);
		if(*buf == '\n')
			sys_putch('\r');
		buf++;
	}
}
/*****************************************************************************
*****************************************************************************/
static void sys_read(unsigned char *buf, unsigned len)
{
	for(; len != 0; len--)
	{
		*buf = getch();
		buf++;
	}
}
/*****************************************************************************
returns nonzero if timeout while waiting for input
*****************************************************************************/
static int sys_select(unsigned *timeout)
{
	unsigned ten_ms;

	for(ten_ms = *timeout / 10; ten_ms != 0; ten_ms--)
	{
		if(kbhit())
		{
			*timeout = ten_ms * 10;
			return 0;
		}
		delay(10);
	}
	return -1;
}
/*****************************************************************************
*****************************************************************************/
static time_t sys_time(void)
{
	return time(NULL);
}
/*****************************************************************************
*****************************************************************************/
static void sys_exit(int err)
{
	longjmp(g_oops, 1 + err);
}
/*****************************************************************************
*****************************************************************************/
int lookup_external_symbol(char *sym_name, unsigned *adr, unsigned uscore)
{
	static struct
	{
		char *name;
		unsigned adr;
	} syms[6] =
	{
		{
			"sys_write", (unsigned)sys_write
		}, {
			"sys_read", (unsigned)sys_read
		}, {
			"sys_select", (unsigned)sys_select
		}, {
			"sys_time", (unsigned)sys_time
		}, {
			"sys_exit", (unsigned)sys_exit
		}
	};
	unsigned i;

/* "initializer element is not constant" */
syms[5].name = "g_sys_ver";
syms[5].adr = (unsigned)&g_sys_ver;
	if(uscore)
	{
		if(sym_name[0] != '_')
			return -1;
		sym_name++;
	}
	for(i = 0; i < sizeof(syms) / sizeof(syms[0]); i++)
	{
		if(!strcmp(syms[i].name, sym_name))
		{
			*adr = syms[i].adr;
			return 0;
		}
	}
	printf("\n\tundefined external symbol '%s'\n", sym_name);
	return -1;
}
