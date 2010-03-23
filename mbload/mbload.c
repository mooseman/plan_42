/*****************************************************************************
MBLOAD-C: a Multiboot-compatible kernel loader
that runs from the DOS command prompt.

Chris Giese	<geezer@execpc.com>	http://my.execpc.com/~geezer
This code is public domain (no copyright).
You can do whatever you want with it.

EXPORTS:
extern struct {} g_mods[];
extern struct g_mboot;
extern unsigned long g_linear, g_phys;
*****************************************************************************/
#include <stdlib.h> /* min(), atexit() */
#include <string.h> /* strncpy(), memcmp(), memset() */
#include <setjmp.h> /* jmp_buf, setjmp(), longjmp() */
#include <stdio.h> /* printf(), sprintf() */

#include <fcntl.h> /* O_RDONLY, O_BINARY */
#include <ctype.h> /* tolower() */

/* MK_FP(), FP_SEG(), FP_OFF(), struct REGPACK, intr(), outportb() */
#include <dirent.h> /* getdrive() */
#include <termios.h> /* SEEK_..., open(), lseek(), tell(), read(), close() */


#define	outportb(P,V)	outp(P,V)
#define	peekb(S,O)	*(unsigned char far *)MK_FP(S,O)

/* leading underscore instead of trailing underscore,
stack calling convention instead of registers */
#pragma aux asm_init	"_*" parm caller [];
#pragma aux xms_exit	"_*" parm caller [];
#pragma aux enter_pmode	"_*" parm caller [];
#pragma aux copy_linear	"_*" parm caller [];

/* WARNING: Watcom C cuserguide.pdf sez structures are packed
by default, but they're not. Compile with -zp1 or use this:
#pragma pack(1) */

/* The nice thing about standards is... */
#define	R_AX	w.ax
#define	R_BX	w.bx
#define	R_SI	w.si
#define	R_DI	w.di
#define	R_ES	w.es
#define	R_DS	w.ds
#define	R_FLAGS	w.flags

typedef union REGPACK	regs_t;

#define	R_AX	r_ax
#define	R_BX	r_bx
#define	R_SI	r_si
#define	R_DI	r_di
#define	R_ES	r_es
#define	R_DS	r_ds
#define	R_FLAGS	r_flags

typedef struct REGPACK	regs_t;

#define _DISK_RESET     0   /* controller hard reset */
#define _DISK_STATUS    1   /* status of last operation */
#define _DISK_READ      2   /* read sectors */
#define _DISK_WRITE     3   /* write sectors */
#define _DISK_VERIFY    4   /* verify sectors */
#define _DISK_FORMAT    5   /* format track */

struct diskinfo_t
{
	unsigned drive, head, track, sector, nsectors;
	void far *buffer;
};

unsigned bios_disk(unsigned cmd, struct diskinfo_t *info)
{
	struct SREGS sregs;
	union REGS regs;

/* biosdisk() returns the 8-bit error code left in register AH by
the call to INT 13h. It does NOT return a combined, 16-bit error
code + number of sectors transferred, as described in the online help.

	return biosdisk(cmd, info->drive, info->head, info->track,
		info->sector, info->nsectors, info->buffer);
*/
	regs.h.ah = cmd;
	regs.h.al = info->nsectors;
	regs.x.bx = FP_OFF(info->buffer);
	regs.h.ch = info->track;
	regs.h.cl = (info->track / 256) * 64 + (info->sector & 0x3F);
	regs.h.dh = info->head;
	regs.h.dl = info->drive;
	sregs.es = FP_SEG(info->buffer);
	int86x(0x13, &regs, &regs, &sregs);
	return regs.x.ax;
}

void dos_getdrive(unsigned *drive)
{
	regs_t regs;

	regs.R_AX = 0x1900; /* AH=19h */
	intr(0x21, &regs);
	*drive = (regs.R_AX & 0xFF) + 1;
}


/* sections/segments in kernel file */
#define	SF_EXEC		0x01	/* executable (code) */
#define	SF_WRITE	0x02	/* writable */
#define	SF_READ		0x04	/* readable */
#define	SF_LOAD		0x08	/* load from disk */
#define	SF_ZERO		0x10	/* zero; do not load from disk */
/* all flags except for SF_ZERO: */
#define	SF_TEXTDATA	(SF_LOAD | SF_READ | SF_WRITE | SF_EXEC)
/* BSS flags */
#define	SF_BSS		(SF_ZERO | SF_READ | SF_WRITE)

/* maximum number of INT 15h AX=E820h memory ranges */
#define	MAX_RANGES	32	/* same as Linux */
/* maximum number of modules */
#define	MAX_MODS	16
/* maximum number of sections/segments in kernel file */
#define	MAX_SECTS	8
/* maximum length of kernel file section/segment name */
#define	SECT_NAME_LEN	16

/* C99 STDINT.H types */
typedef unsigned char	uint8_t; /* 8-bit */
typedef unsigned short	uint16_t; /* 16-bit */
typedef unsigned long	uint32_t; /* 32-bit */

/* section or segment inside executable file */
typedef struct
{
	char name[SECT_NAME_LEN];
	unsigned long adr, size, offset;
	unsigned char flags;
} sect_t;

/* ELF files */
#pragma pack(1)
typedef struct
{
	unsigned char magic[4];
	unsigned char bitness;
	unsigned char endian;
	unsigned char elf_ver_1;
	unsigned char res[9];
	uint16_t file_type;
	uint16_t machine;
	uint32_t elf_ver_2;
	uint32_t entry;
	uint32_t phtab_offset;
	uint32_t shtab_offset;
	uint32_t flags;
	uint16_t file_hdr_size;
	uint16_t ph_size;
	uint16_t num_phs;
	uint16_t sh_size;
	uint16_t num_sects;
	uint16_t shstrtab_index;
} elf_file_t; /* 52 bytes */

typedef struct
{
	uint32_t type;
	uint32_t offset;
	uint32_t virt_adr;
	uint32_t phys_adr;
	uint32_t disk_size;
	uint32_t mem_size;
	uint32_t flags;
	uint32_t align;
} elf_seg_t; /* 32 bytes */

/* DOS 16-bit .EXE file */
typedef struct
{
	char magic[2];
/* not really unused, but we don't care about them for PE */
	char unused[58];
	uint32_t new_exe_offset;
} exe_file_t;

/* Win32 PE and DJGPP COFF files */
typedef struct
{
	uint16_t magic;
	uint16_t num_sects;
	uint32_t time_date;
	uint32_t symtab_offset;
	uint32_t num_syms;
	uint16_t aout_size;
	uint16_t file_flags;
} coff_file_t; /* 20 bytes */

typedef struct
{
	uint16_t magic;
	uint16_t version;
	uint32_t code_size;
	uint32_t data_size;
	uint32_t bss_size;
	uint32_t entry;
	uint32_t code_offset;
	uint32_t data_offset;
} dj_aout_t; /* 28 bytes */

typedef struct
{
	uint16_t magic;
	uint16_t version;
	uint32_t code_size;
	uint32_t data_size;
	uint32_t bss_size;
	uint32_t entry;
	uint32_t code_offset;
	uint32_t data_offset;
	uint32_t image_base;
	uint32_t res0[18];
	uint32_t import_table_adr;
	uint32_t import_table_size;
} pe_aout_t;

typedef struct
{
	char name[8];
	uint32_t phys_adr;
	uint32_t virt_adr;
	uint32_t size;
	uint32_t offset;
	uint32_t relocs_offset;
	uint32_t line_nums_offset;
	uint16_t num_relocs;
	uint16_t num_line_nums;
	uint32_t flags;
} dj_sect_t; /* 40 bytes */

typedef struct
{
	char name[8];
	uint32_t virt_size;	 /* size-in-mem */
	uint32_t virt_adr;	 /* RVA */
	uint32_t raw_size;	 /* size-on-disk */
	uint32_t offset;
	uint32_t relocs_offset;
	uint32_t line_nums_offset;
	uint16_t num_relocs;
	uint16_t num_line_nums;
	uint32_t flags;
} pe_sect_t;

/* Multiboot stuff:
Header placed in kernel file to make it compatible with Multiboot */
typedef struct
{
	uint32_t magic;
	uint32_t flags;
	uint32_t checksum;
	uint32_t hdr_adr;
	uint32_t load_adr;
	uint32_t bss_adr;
	uint32_t end_adr;
	uint32_t entry;
} mboot_hdr_t;

/* bits in flags field of Multiboot struct (see below) */
#define MBF_MEMORY	0x00000001
#define MBF_ROOTDEV	0x00000002
#define MBF_CMDLINE	0x00000004
#define MBF_MODS	0x00000008
#define MBF_AOUT_SYMS	0x00000010
#define MBF_ELF_SHDR	0x00000020
#define MBF_MEM_MAP	0x00000040
#define MBF_DRIVE_INFO	0x00000080
#define MBF_CFG_TABLE	0x00000100
#define MBF_LOADER_NAME	0x00000200
#define MBF_APM_TABLE	0x00000400
#define MBF_VBE_INFO	0x00000800

/* This is the Multiboot struct full of system info. A pointer to this
struct is passed to the kernel in the EBX register. We don't support
all the bells and whistles that GRUB does. */
typedef struct
{
	uint32_t flags;
	uint32_t conv_mem;		/* MBF_MEMORY */
	uint32_t ext_mem;		/* " */
	uint32_t root_dev;		/* MBF_ROOTDEV */
	uint32_t cmd_line;		/* MBF_CMDLINE */
/* modules */
	uint16_t num_mods;		/* MBF_MODS */
	uint16_t unused2;
	uint32_t mods_adr;		/* " */
/* no symbol table (MBF_AOUT_SYMS) or section table (MBF_ELF_SHDR) */
	uint32_t unused3[4];
/* BIOS memory map */
	uint32_t map_len;		/* MBF_MEM_MAP */
	uint32_t map_adr;		/* " */
/* info about hard drives */
	uint32_t drives_len;		/* MBF_DRIVE_INFO */
	uint32_t drives_adr;		/* " */
/* ROM config table */
	uint32_t config_table;		/* MBF_CFG_TABLE */
/* boot loader name */
	uint32_t loader_name;		/* MBF_LOADER_NAME */
/* APM table */
	uint32_t apm_table;		/* MBF_APM_TABLE */
/* video info */
	uint32_t vbe_ctrl_info;		/* MBF_VBE_INFO */
	uint32_t vbe_mode_info;		/* " */
	uint16_t vbe_mode;		/* " */
	uint16_t vbe_iface_seg;		/* " */
	uint16_t vbe_iface_off;		/* " */
	uint16_t vbe_iface_len;		/* " */
} mboot_info_t;

/* Multiboot modules */
typedef struct
{
	uint32_t start_adr;
	uint32_t end_adr;
	uint32_t cmd_line;
	uint32_t unused;
} mboot_mod_t;

/* Multiboot BIOS memory ranges */
typedef struct
{
	uint32_t len;		 /* =24=size of this struct */
	uint32_t adr, res_adr;
	uint32_t size, res_size;
	uint16_t type, res_type;
} mboot_range_t;

typedef struct
{
	uint32_t len;
	uint8_t drive_num;	 /* INT 13h drive number */
	uint8_t use_lba;
/* drive geometry values if use_lba==0 */
	uint16_t cyls;
	uint8_t heads;
	uint8_t sects;
/* ...I/O port fields can go here; adjust len accordingly... */
} mboot_drive_t;

mboot_info_t g_mboot;
mboot_mod_t g_mods[MAX_MODS];
/* The kernel has three addresses of interest:
- lowest virtual address; computed in load_kernel()
- desired physical (load) address of kernel == g_phys
- initial load address of kernel == g_linear
g_phys != g_linear because of XMS. In this case, we will copy the loaded
kernel and modules from g_linear to g_phys just before entering pmode. */
unsigned long g_linear, g_phys;

/* is entry point a virtual address or physical? */
static char g_virt_entry;
/* protected mode kernel? (always =1 for this version of MBLOAD) */
static char g_pmode;
/* sections/segments in kernel file */
static unsigned g_num_sects;
static sect_t g_sects[MAX_SECTS];
/* kernel file format */
static char *g_krnl_format = "*** UNKNOWN ***";
/* "bounce buffer"; for zeroing or loading things to extended memory.
Size of this buffer must be even. */
static char g_buffer[4096];

static jmp_buf g_oops;
static char g_bad_kernel;

/* IMPORTS
from LIB.ASM */
extern mboot_range_t g_ranges[];
extern unsigned char g_num_ranges;
extern unsigned long g_entry;
extern char g_cpu32, g_v86, g_dos, g_xms;
/* conventional memory (< 1 meg) and extended memory (>= 1 meg) heaps */
extern unsigned long g_extmem_adr, g_extmem_size;
extern unsigned long g_convmem_adr, g_convmem_size;

void asm_init(void);
void xms_exit(void);
void enter_pmode(void);
/* copies 'count' bytes (< 64K) between linear addresses */
int copy_linear(long dst_linear, long src_linear, unsigned count);
/*****************************************************************************
Adds executable file section to global list defined by
g_num_sects and g_sects. The new section is described by
the arguments to this function (name, adr, size, etc.)
*****************************************************************************/
static void add_section(const char *name, unsigned long adr,
		unsigned long size, unsigned long offset, unsigned flags)
{
	static unsigned long prev_adr;
/**/
	sect_t *sect;

	if(g_num_sects >= MAX_SECTS)
	{
		printf("*** Too many (>%u) sections/segments in kernel\n",
			MAX_SECTS);
		g_bad_kernel = 1;
		return;
	}
	sect = g_sects + g_num_sects;
/* if no section name (e.g. for ELF segments),
generate name from section number */
	if(name == NULL)
		sprintf(sect->name, "(%u)", g_num_sects);
	else
		strncpy(sect->name, name, SECT_NAME_LEN - 1);
	sect->name[SECT_NAME_LEN - 1] = '\0';
/* ### - hacks to detect no-.rodata-in-linker-script bug;
a common error with ELF kernels (.rodata is still copied to
the output executable, but ld places it at address 0) */
	if(adr < 0x100000uL)
	{
		printf("*** Address of section/segment '%s' is below "
			"1 meg\n", sect->name);
		g_bad_kernel = 1;
	}
	if(prev_adr != 0 && adr < prev_adr)
	{
		printf("*** Address of section/segment '%s' is lower "
			"than a previous section\n", sect->name);
		g_bad_kernel = 1;
	}
	prev_adr = adr;

	sect->adr = adr;
	sect->size = size;
	sect->offset = offset;
	sect->flags = flags;
	g_num_sects++;
}
/*****************************************************************************
Debug dump of global list of executable file sections
*****************************************************************************/
static void dump_sections(void)
{
	sect_t *sect;
	unsigned i;

	printf(	"          Section  Address   Size Offset Flags\n"
		"    ------------- -------- ------ ------ -----\n");
	sect = g_sects + 0;
	for(i = 0; i < g_num_sects; i++, sect++)
	{
		printf("    %13.13s %8lX %6lX %6lX ", sect->name,
			sect->adr, sect->size, sect->offset);
		putchar((sect->flags & SF_ZERO) ? 'Z' : '-');
		putchar((sect->flags & SF_LOAD) ? 'L' : '-');
		putchar((sect->flags & SF_READ) ? 'R' : '-');
		putchar((sect->flags & SF_WRITE) ? 'W' : '-');
		putchar((sect->flags & SF_EXEC) ? 'X' : '-');
		putchar('\n');
	}
}
/*****************************************************************************
*****************************************************************************/
static void read_or_die(int f, void *buf, unsigned count)
{
	if(read(f, buf, count) != count)
		longjmp(g_oops, (int)"Error reading file (unexpected EOF)");
}
/*****************************************************************************
*****************************************************************************/
static int open_or_die(const char *name)
{
	int f;

	f = open(name, O_RDONLY | O_BINARY);
	if(f < 0)
	{
		sprintf(g_buffer, "Can't open file '%s'", name);
		longjmp(g_oops, (int)g_buffer);
	}
	return f;
}
/*****************************************************************************
Checks if file is ELF executable. If it is, reads and stores VIRTUAL
addresses (VMAs; not LMAs) of PROGRAM HEADERS (segments; not sections).
Sets load address (g_phys) to lowest PHYSICAL segment address.
*****************************************************************************/
static int is_elf(int f)
{
	unsigned i, flags, first_seg = 1;
	unsigned long virt_to_phys;
	elf_file_t file;
	elf_seg_t seg;

/* seek to start of file */
	lseek(f, 0, SEEK_SET);
/* read and validate file headers */
	if(read(f, &file, sizeof(elf_file_t)) != sizeof(elf_file_t) ||
		file.magic[0] != '\x7F' || file.magic[1] != 'E' ||
		file.magic[2] != 'L' || file.magic[3] != 'F')
			return 0;
/* validated -- any badness from here on is an error */
	if(file.bitness != 1 ||		/* 32-bit */
		file.endian != 1 ||	/* little-endian */
		file.elf_ver_1 != 1 ||
		file.file_type != 2 ||	/* executable */
		file.machine != 3 ||	/* i386 */
		file.elf_ver_2 != 1)
			longjmp(g_oops, (int)"ELF file has "
				"invalid file headers");
	g_virt_entry = 1;
	g_pmode = 1;
	g_krnl_format = "ELF";
/* get VIRTUAL entry point */
	g_entry = file.entry;
	g_phys = -1uL;
/* read program headers (segments) */
	for(i = 0; i < file.num_phs; i++)
	{
		lseek(f, file.phtab_offset +
			file.ph_size * i, SEEK_SET);
		read_or_die(f, &seg, sizeof(elf_seg_t));
/* choke on 2=DYNAMIC and the forbidden 5=SHLIB segments */
		if(seg.type == 2)
		{
			printf("*** ELF kernel is dynamically-linked\n");
			g_bad_kernel = 1;
		}
		else if(seg.type == 5)
		{
/* Mmmmm...forbidden segment... */
			printf("*** ELF kernel contains forbidden segment\n");
			g_bad_kernel = 1;
		}
/* handle 1=LOAD segment */
		else if(seg.type == 1)
		{
/* make sure all segments have the same virt-to-phys value */
			if(first_seg)
			{
				virt_to_phys = seg.phys_adr - seg.virt_adr;
				first_seg = 0;
			}
			else if(virt_to_phys != seg.phys_adr - seg.virt_adr)
			{
				printf("*** Segments in ELF kernel have "
					"different virt-to-phys values\n");
				g_bad_kernel = 1;
			}
/* store lowest physical segment adr as g_phys */
			if(seg.phys_adr < g_phys)
				g_phys = seg.phys_adr;
			flags = SF_LOAD | (unsigned)(seg.flags & 7);
/* get segment VIRTUAL addresses */
			add_section(NULL, seg.virt_adr,
				seg.disk_size, seg.offset, flags);
/* if size-in-mem > size-on-disk, this segment contains the BSS */
			if(seg.mem_size <= seg.disk_size)
				continue;
			flags = SF_ZERO | (unsigned)(seg.flags & 7);
			add_section(NULL, seg.virt_adr + seg.disk_size,
				seg.mem_size - seg.disk_size, 0, flags);
		}
/* ignore 0=NULL, 6=PHDR, 3=INTERP, and 4=NOTE segments
		else
			nothing; */
	}
	return 1;
}
/*****************************************************************************
Checks if file is Multiboot-ELF or Multiboot-kludge.
Builds global section list, as with is_elf()
*****************************************************************************/
static int is_mboot(int f)
{
	mboot_hdr_t mboot;
	elf_file_t file;
	int hdr_off;

/* scan first 8K of file for Multiboot magic value */
	for(hdr_off = 0; hdr_off < 8192; hdr_off += 4)
	{
		lseek(f, hdr_off, SEEK_SET);
		if(read(f, &mboot, sizeof(mboot_hdr_t)) != sizeof(mboot_hdr_t))
			return 0;
		if(mboot.magic != 0x1BADB002uL)
			continue;
		if(mboot.magic + mboot.flags + mboot.checksum == 0)
			goto OK;
	}
	return 0;
OK:
/* check if file is ELF */
	lseek(f, 0, SEEK_SET);
	if(read(f, &file, sizeof(elf_file_t)) == sizeof(elf_file_t) &&
		file.magic[0] == '\x7F' && file.magic[1] == 'E' &&
		file.magic[2] == 'L' && file.magic[3] == 'F')
/* it's Multiboot-ELF */
	{
		(void)is_elf(f);
/* change format name from ELF to Multiboot-ELF */
		g_krnl_format = "Multiboot-ELF";
		if(mboot.flags & 0x00010000uL)
			printf("*** Warning: kludge fields ignored "
				"for Multiboot-ELF kernel\n");
		return 1;
	}
/* not ELF -- kludge flag must be set */
	if((mboot.flags & 0x00010000uL) == 0)
#if 1
	{
		printf("*** Warning: kludge fields required for "
			"non-ELF Multiboot kernels\n");
/* try DJGPP COFF or other format
this is a feature of geezerboot, not Multiboot :) */
		return 0;
	}
#else
		longjmp(g_oops, (int)"Non-ELF Multiboot kernel "
			"requires kludge fields");
#endif
	g_virt_entry = 0;
	g_pmode = 1;
	g_krnl_format = "Multiboot-kludge";
/* get PHYSICAL entry point */
	g_entry = mboot.entry;
	g_phys = mboot.load_adr;
/* text-and-data section */
	add_section("text-and-data", mboot.load_adr,
		mboot.bss_adr - mboot.load_adr,
		hdr_off - mboot.hdr_adr + mboot.load_adr, SF_TEXTDATA);
/* BSS */
	add_section("bss", mboot.bss_adr, mboot.end_adr - mboot.bss_adr,
		0, SF_BSS);
	return 1;
}
/*****************************************************************************
Checks if file is non-Multiboot DJGPP COFF, and builds global section list
*****************************************************************************/
static int is_djcoff(int f)
{
	struct
	{
		coff_file_t coff;
		dj_aout_t aout;
	} hdr;
	dj_sect_t sect;
	int i, flags;

/* seek to start of file */
	lseek(f, 0, SEEK_SET);
/* read and validate file headers */
	if(read(f, &hdr, sizeof(hdr)) != sizeof(hdr) ||
		hdr.coff.magic != 0x014C || hdr.coff.aout_size != 28 ||
		hdr.aout.magic != 0x010B)
			return 0;
/* validated -- any badness from here on is an error */
	g_virt_entry = 1;
	g_pmode = 1;
	g_krnl_format = "DJGPP COFF";
	g_phys = 0x100000L; /* 1 meg */
/* get entry point */
	g_entry = hdr.aout.entry;
/* read section headers */
	for(i = 0; i < hdr.coff.num_sects; i++)
	{
		read_or_die(f, &sect, sizeof(sect));
/* code */
		if((sect.flags & 0xE0) == 0x20)
			flags = SF_LOAD | SF_READ | SF_EXEC;
/* data */
		else if((sect.flags & 0xE0) == 0x40)
			flags = SF_LOAD | SF_READ | SF_WRITE;
/* BSS */
		else if((sect.flags & 0xE0) == 0x80)
			flags = SF_BSS;
/* ignore anything else */
		else
			continue;
		add_section(sect.name, sect.virt_adr, sect.size,
			sect.offset, flags);
	}
	return 1;
}
/*****************************************************************************
Checks if file is non-Multiboot Win32 PE COFF, and builds global section list
*****************************************************************************/
static int is_pecoff(int f)
{
	unsigned long bss_adr, bss_size;
	exe_file_t exe;
	struct
	{
		unsigned char pe[4];
		coff_file_t coff;
		pe_aout_t aout;
	} hdr;
	char saw_bss = 0;
	pe_sect_t sect;
	int i, flags;

/* seek to start of file */
	lseek(f, 0, SEEK_SET);
/* read and validate file headers */
	if(read(f, &exe, sizeof(exe)) != sizeof(exe) ||
		exe.magic[0] != 'M' || exe.magic[1] != 'Z')
			return 0;
	lseek(f, exe.new_exe_offset, SEEK_SET);
	if(read(f, &hdr, sizeof(hdr)) != sizeof(hdr) || hdr.pe[0] != 'P' ||
		hdr.pe[1] != 'E' || hdr.pe[2] != 0 || hdr.pe[3] != 0 ||
		hdr.coff.magic != 0x014C || hdr.coff.aout_size != 224 ||
		hdr.aout.magic != 0x010B)
			return 0;
/* validated -- any badness from here on is an error */
	g_virt_entry = 1;
	g_pmode = 1;
	g_krnl_format = "Win32 PE COFF";
	g_phys = 0x100000L; /* 1 meg */
/* dynamically-linked? */
	if(hdr.aout.import_table_size != 0)
	{
		printf("*** PE kernel is dynamically-linked\n");
		g_bad_kernel = 1;
	}
/* get entry point */
	g_entry = hdr.aout.entry + hdr.aout.image_base;
/* read section headers */
	for(i = 0; i < hdr.coff.num_sects; i++)
	{
		lseek(f, exe.new_exe_offset + 248 +
			sizeof(sect) * i, SEEK_SET);
		read_or_die(f, &sect, sizeof(sect));
/* code */
		if(!memcmp(sect.name, ".text", 5) &&
			(sect.flags & 0xE0) == 0x20)
				flags = SF_LOAD | SF_READ | SF_EXEC;
/* data */
		else if(!memcmp(sect.name, ".data", 5) &&
			(sect.flags & 0xE0) == 0x40)
		{
			flags = SF_LOAD | SF_READ | SF_WRITE;
/* if BSS is part of .data... */
			bss_adr = hdr.aout.image_base +
				sect.virt_adr + sect.raw_size;
			bss_size = sect.virt_size - sect.raw_size;
		}
/* BSS */
		else if(!memcmp(sect.name, ".bss", 4) &&
			(sect.flags & 0xE0) == 0x80)
		{
			sect.raw_size = sect.virt_size;
			flags = SF_BSS;
			saw_bss = 1;
		}
/* ignore anything else */
		else
			continue;
		add_section(sect.name, hdr.aout.image_base +
			sect.virt_adr, sect.raw_size, sect.offset, flags);
	}
	if(!saw_bss)
		add_section(".bss", bss_adr, bss_size, 0, SF_BSS);
	return 1;
}
/*****************************************************************************
Builds conventional and extended memory heaps from BIOS memory map
*****************************************************************************/
static void init_heap(void)
{
	mboot_range_t *range;
	unsigned i;

/* if DOS, g_convmem_size and g_convmem_adr already set by asm code
For this version of MBLOAD, g_dos is always true. */
	if(!g_dos)
	{
		range = g_ranges + 0;
		for(i = 0; i < g_num_ranges; i++, range++)
		{
/* ignore non-RAM ranges */
			if(range->type != 1)
				continue;
/* ignore range outside conventional memory */
			if(range->adr + range->size > 0x100000uL)
				continue;
/* use the biggest contiguous range in conventional memory for the heap */
			if(range->size > g_convmem_size)
			{
				g_convmem_size = range->size;
				g_convmem_adr = range->adr;
			}
		}
/* subtract conventional memory used by this code
		### */
	}
/* if XMS, g_extmem_size and g_extmem_adr already set by asm code */
	if(!g_xms)
	{
		range = g_ranges + 0;
		for(i = 0; i < g_num_ranges; i++, range++)
		{
/* ignore non-RAM ranges */
			if(range->type != 1)
				continue;
/* ignore range outside extended memory */
			if(range->adr < 0x100000uL)
				continue;
			if(range->size > g_extmem_size)
			{
				g_extmem_size = range->size;
				g_extmem_adr = range->adr;
			}
		}
	}
}
/*****************************************************************************
Allocates conventional (high == 0) or extended (high != 0) memory block
of 'size' bytes. Size will be rounded up to next paragraph for
conventional memory, to next page for extended memory.
Returns linear address of block.
*****************************************************************************/
static unsigned long alloc(unsigned long size, char high)
{
	unsigned long rv;

	if(high)
	{
/* round to page (4K) boundary */
		size = (size + 4095) & 0xFFFFF000uL;
		if(size > g_extmem_size)
			longjmp(g_oops, (int)"Out of extended/XMS memory");
		rv = g_extmem_adr;
		g_extmem_adr += size;
		g_extmem_size -= size;
	}
	else
	{
/* round to paragraph (16-byte) boundary */
		size = (size + 15) & 0xFFFFFFF0uL;
		if(size > g_convmem_size)
			longjmp(g_oops, (int)"Out of conventional memory");
		rv = g_convmem_adr;
		g_convmem_adr += size;
		g_convmem_size -= size;
	}
	return rv;
}
/*****************************************************************************
Converts 16:16 far pointer to 32-bit linear address
*****************************************************************************/
static unsigned long to_linear(void far *adr)
{
	return 16uL * FP_SEG(adr) + FP_OFF(adr);
}
/*****************************************************************************
Reads 'size' bytes from current position in 'file' to linear address
'linear', which may be in conventional or extended memory.
*****************************************************************************/
static void read_linear(int f, unsigned long linear, unsigned long size)
{
	char huge *dst;
	int i, j;

	if(linear < 0x100000uL)
	{
/* can't happen -- can it? */
		if(linear + size >= 0x100000uL)
			longjmp(g_oops, (int)"Software error: "
				"reading across 1 MB");
/* damn compiler bugs!
		dst = MK_FP(linear / 16, linear & 0x0F); */
		i = (unsigned)linear & 0x0F;
		linear >>= 4;
		dst = MK_FP(linear, i);
/* the only way to avoid a loop here is to have a read() function
that A) reads to a 'far' buffer, and B) has a 32-bit count value --
impossible with DOS (see INT 21h AH=3Fh) */
		while(size != 0)
		{
			i = (unsigned)min(size, sizeof(g_buffer));
/* read to a block of conventional memory... */
			read_or_die(f, g_buffer, i);
/* ...then copy it to conventional memory range */
			for(j = 0; j < i; j++)
			{
				*dst = g_buffer[j];
				dst++;
			}
			size -= i;
		}
	}
	else
	{
		while(size != 0)
		{
			i = (unsigned)(min(size, sizeof(g_buffer)));
/* read to "bounce buffer", in conventional memory... */
			read_or_die(f, g_buffer, i);
/* ...then copy it to extended memory range
round up count to next even value */
			if(copy_linear(linear, to_linear(g_buffer),
				(i + 1) & ~1) != 0)
					longjmp(g_oops,
					(int)"Error copying to "
					"extended/XMS memory");
			size -= i;
			linear += i;
		}
	}
}
/*****************************************************************************
Zeroes 'size' bytes at linear address 'linear',
which may be in conventional or extended memory
*****************************************************************************/
static void zero_linear(unsigned long linear, unsigned long size)
{
	char huge *dst;
	unsigned i;

	if(linear < 0x100000uL)
	{
/* can't happen? */
		if(linear + size >= 0x100000uL)
			longjmp(g_oops, (int)"Software error: "
				"zeroing across 1 MB");
/* zero conventional memory range */
	/*	dst = MK_FP(linear / 16, linear & 0x0F); */
		i = (unsigned)(linear & 0x0F);
		linear >>= 4;
		dst = MK_FP(linear, i);
		for(; size != 0; size--)
		{
			*dst = 0;
			dst++;
		}
	}
	else
	{
/* zero a block of conventional memory... */
		memset(g_buffer, 0, sizeof(g_buffer));
/* ...then copy it to extended memory range */
		while(size != 0)
		{
			i = (unsigned)min(size, sizeof(g_buffer));
/* round up count to next even value */
			if(copy_linear(linear, to_linear(g_buffer),
				(i + 1) & ~1) != 0)
					longjmp(g_oops,
					(int)"Error copying to "
					"extended/XMS memory");
			size -= i;
			linear += i;
		}
	}
}
/*****************************************************************************
Loads 16-bit real mode kernel or 32-bit pmode kernel. The global list
of sections in the kernel file, defined by 'g_num_sects' and 'g_sects',
must be set before calling this function.

g_phys = where the kernel "wants" to be loaded, e.g. 1 meg
g_linear = where the kernel is initially loaded.
These addresses may be different if XMS is present.

If g_phys != g_linear, the kernel will be copied
from g_linear to g_phys just before entering pmode
*****************************************************************************/
static void load_kernel(int f)
{
	unsigned long lowest, highest, extent, virt_to_phys;
	sect_t *sect;
	int i;

/* find lowest and highest addresses */
	lowest = -1uL;
	highest = 0uL;
	sect = g_sects + 0;
	for(i = 0; i < g_num_sects; i++, sect++)
	{
		if(sect->adr < lowest)
			lowest = sect->adr;
		if(sect->adr + sect->size > highest)
			highest = sect->adr + sect->size;
	}
/* highest - lowest = kernel "extent"
(true size of kernel in memory, including any gaps between sections) */
	extent = highest - lowest;
/* allocate conventional memory for real-mode kernel,
extended memory for pmode kernel */
	g_linear = alloc(extent, g_pmode);
	printf("    0x%lX bytes at 0x%lX; lowest virtual address 0x%lX\n",
		extent, g_linear, lowest);
/* form virtual-to-physical address conversion value from lowest address
and initial load address */
	virt_to_phys = g_linear - lowest;
/* for each section... */
	sect = g_sects + 0;
	for(i = 0; i < g_num_sects; i++, sect++)
	{
/* zero the BSS */
		if(sect->flags & SF_ZERO)
			 zero_linear(sect->adr + virt_to_phys, sect->size);
/* load other sections */
		else
		{
			lseek(f, sect->offset, SEEK_SET);
			read_linear(f, sect->adr + virt_to_phys, sect->size);
		}
	}
/* convert entry point to physical address */
	if(g_virt_entry)
{ printf("g_virt_entry: old g_entry=0x%lX, g_phys=0x%lX, lowest=0x%lX, ",
 g_entry, g_phys, lowest);
/* whoops...fixed in version 0.50
		g_entry += virt_to_phys; */
		g_entry += (g_phys - lowest);
printf("new g_entry=0x%lX\n", g_entry);
}
}
/*****************************************************************************
For each DOS drive letter, maps drive to INT 13h drive number (if possible)
*****************************************************************************/
#define	MAX_DRIVES	26

static struct
{
	char letter;
	unsigned char drive, part;
} g_drives[MAX_DRIVES];

static unsigned g_num_drives;

static void init_drives(void)
{
	typedef struct _ddt
	{
		struct _ddt far *next;
		unsigned char drive_num;
		unsigned char drive_letter;
		char res[17]; /* remainder of BPB */
		unsigned long hidden_sects;
		/* ...more stuff here we don't care about... */
	} ddt_t;
/**/
	int curr_drive = -1, part;
	struct diskinfo_t cmd;
	char buf[512], *pte;
	ddt_t far *ddt;
	regs_t regs;

/* check if DRIVER.SYS installed (always present for MS-DOS 3.2+)
Fixed in version 0.50: FreeDOS supports INT 2Fh AX=0803h but not
INT 2Fh AX=0800h, so skip this check.
	regs.R_AX = 0x0800;
	intr(0x2F, &regs);
	if((regs.R_AX & 0xFF) != 0xFF)
		longjmp(g_oops, (int)"DRIVER.SYS not present (FreeDOS?)"); */
	cmd.head = cmd.track = 0;
	cmd.sector = cmd.nsectors = 1;
	cmd.buffer = buf;
/* scan DRIVER.SYS chain */
	printf("    DOS-to-GRUB drive mappings:\n  ");
	regs.R_AX = 0x0803;
	intr(0x2F, &regs);
	ddt = (ddt_t far *)MK_FP(regs.R_DS, regs.R_DI);
	for(; FP_OFF(ddt) != 0xFFFF; ddt = ddt->next)
	{
		if(g_num_drives >= MAX_DRIVES)
		{
			printf("\nToo many (>=%u) drives\n", MAX_DRIVES);
			break;
		}
		g_drives[g_num_drives].letter = ddt->drive_letter + 'A';
		g_drives[g_num_drives].drive = ddt->drive_num;
/* floppy drive */
		if(ddt->drive_num < 0x80)
		{
			printf("    %c: = (fd%u)",
				ddt->drive_letter + 'A', ddt->drive_num);
			g_num_drives++;
			continue;
		}
/* hard drive. Load MBR/partition table */
		if(ddt->drive_num != curr_drive)
		{
			cmd.drive = ddt->drive_num;
			if(_bios_disk(_DISK_READ, &cmd) >= 0x100)
			{
				_bios_disk(_DISK_RESET, &cmd);
				if(_bios_disk(_DISK_READ, &cmd) >= 0x100)
				{
					printf("\nError reading drive 0x%02X\n",
						ddt->drive_num);
					continue;
				}
			}
			curr_drive = ddt->drive_num;
		}
/* scan partition table for matching partition (same starting sector) */
		for(part = 0; part < 4; part++)
		{
			pte = buf + 446 + 16 * part;
			if(*(long *)(pte + 8) == ddt->hidden_sects)
			{
				printf("  %c: = (hd%u,%u)",
					ddt->drive_letter + 'A',
					ddt->drive_num - 0x80, part);
				g_drives[g_num_drives].part = part;
				g_num_drives++;
				goto OK;
			}
		}
		printf("\nIgnoring weird hard drive partition %c:\n",
			ddt->drive_letter + 'A');
OK:		;
	}
	printf("\n");
}
/*****************************************************************************
Converts DOS drive letter to INT 13h drive number and
(for hard drives only) partition number.
*****************************************************************************/
static int get_drive_num(unsigned *drive, unsigned *part, char drive_letter)
{
	unsigned i;

	for(i = 0; i < g_num_drives; i++)
	{
		if(g_drives[i].letter == drive_letter)
		{
			*drive = g_drives[i].drive;
			*part = g_drives[i].part;
			return 0;
		}
	}
	return -1;
}
/*****************************************************************************
Converts DOS-style path name, e.g.	C:..\FOO.EXE
to GRUB-style path name, e.g.		(hd0,1)/my-os/bin/foo.exe
*****************************************************************************/
static void convert_name(char **path_p, char *dos_name)
{
	unsigned drive, part;
	char *path, *s;
	regs_t regs;

/* use undocumented DOS TRUENAME call to get canonical full path to file */
	regs.R_AX = 0x6000; /* AH=60h */
	regs.R_DS = FP_SEG(dos_name);
	regs.R_SI = FP_OFF(dos_name);
	regs.R_ES = FP_SEG(g_buffer);
	regs.R_DI = FP_OFF(g_buffer);
	intr(0x21, &regs);
	if(regs.R_FLAGS & 0x0001)
MERR:	{
		sprintf(g_buffer, "Can't get full path to file '%s' "
			"(TRUENAME call failed)", dos_name);
		longjmp(g_oops, (int)g_buffer);
	}
/* convert DOS drive letter at start of path to INT 13h drive num and part */
	if(get_drive_num(&drive, &part, g_buffer[0]))
		goto MERR;
/* allocate memory for GRUB-style path
-2 bytes for DOS drive letter and colon, e.g. "C:"
+11 bytes for (hdMMM,NNN)
+1 for trailing '\0' */
	path = malloc(strlen(g_buffer) - 2 + 11 + 1);
	if(path == NULL)
		longjmp(g_oops, (int)"Out of memory");
/* build GRUB-style path */
	if(drive < 0x80)
		sprintf(path, "(fd%u)%s", drive, g_buffer + 2);
	else
		sprintf(path, "(hd%u,%u)%s", drive - 0x80,
			part, g_buffer + 2);
/* convert '\' to '/', and make everything lower-case */
	for(s = path; *s != '\0'; s++)
	{
		if(*s == '\\')
			*s = '/';
		else
			*s = tolower(*s);
	}
	(*path_p) = path;
}
/*****************************************************************************
*****************************************************************************/
int main(int arg_c, char *arg_v[])
{
	char *file_name, *path, *err;
	unsigned long linear, len;
	unsigned i, drive, part;
	mboot_range_t *range;
	regs_t regs;
	int f = -1;

/* do the things that are difficult or impossible in C... */
	asm_init();
/* ...but remember to un-do them if we exit.
This is mainly to unlock and free XMS memory */
	atexit(xms_exit);
/* set up error trapping */
	err = (char *)setjmp(g_oops);
	if(err != NULL)
	{
		printf("*** %s\n", err);
		if(f != -1)
		{
			close(f);
			f = -1;
		}
		return 1;
	}
/* build low (conventional memory) and high (extended memory) heaps */
	init_heap();
/* DISPLAY SYSTEM INFO */
	printf("SYSTEM:\n");
	printf("    BIOS memory ranges:\n");
	for(i = 0; i < g_num_ranges; i++)
		printf("\tType=%u, base adr=0x%8lX, size=0x%8lX\n",
			g_ranges[i].type, g_ranges[i].adr, g_ranges[i].size);
	printf("    %luK conventional memory at 0x%lX, "
		"%luK %s memory at 0x%lX\n", g_convmem_size / 1024,
		g_convmem_adr, g_extmem_size / 1024,
		g_xms ? "locked XMS" : "extended", g_extmem_adr);
	printf("    DOS: %s    32-bit CPU: %s    V86 mode: %s    XMS: %s\n",
		(g_dos ? "yes" : "no"), (g_cpu32 ? "yes" : "no"),
		(g_v86 ? "yes" : "no"), (g_xms ? "yes" : "no"));
/* map DOS drive letters to GRUB drive names */
	init_drives();
/* LOAD KERNEL:
get kernel file name */
	if(arg_c < 2)
		longjmp(g_oops, (int)"Loads Multiboot kernels "
			"and optional modules. Usage:\n"
			"mbload kernel [module module ... module]\n");
	file_name = arg_v[1];
	printf("KERNEL:\n    File name '%s'\n", file_name);
/* store full GRUB-style path to kernel in g_mboot */
	convert_name(&path, file_name);
	g_mboot.cmd_line = to_linear(path);
	g_mboot.flags |= MBF_CMDLINE;
/* open kernel file */
	f = open_or_die(file_name);
/* check file format and get section/segment info */
	if(is_mboot(f))
		/* nothing */;
	else if(is_elf(f))
/* for plain (non-Multiboot) ELF, set load address to 1 meg */
		g_phys = 0x100000L; /* 1 meg */
	else if(is_djcoff(f))
		/* nothing */;
	else if(is_pecoff(f))
		/* nothing */;
	else
		longjmp(g_oops, (int)"Unknown kernel file format");
/* otherwise, DISPLAY KERNEL INFO */
	printf("    File format '%s', pmode: %s\n",
		g_krnl_format, (g_pmode ? "yes" : "no"));
	printf("    Entry point 0x%lX, load address 0x%lX\n",
		g_entry, g_phys);
	dump_sections();
/* still more diagnostics */
	if(g_pmode)
	{
		if(!g_cpu32)
			longjmp(g_oops, (int)"32-bit CPU (386SX+) required");
		if(g_v86)
			longjmp(g_oops, (int)"CPU is in V86 mode "
				"(Windows DOS box or EMM386 loaded?)");
	}
	if(g_bad_kernel)
	{
		close(f);
		f = -1;
		return 1;
	}
/* load real- or protected-mode kernel
UI note: load_kernel() also prints kernel info */
	load_kernel(f);
/* close kernel file */
	close(f);
	f = -1;
/* LOAD MODULE(S) */
	printf("MODULES:\n");
	for(i = 2; i < arg_c; i++)
	{
		if(g_mboot.num_mods >= MAX_MODS)
		{
			printf("*** Warning: Too many (>%u) modules\n",
				MAX_MODS);
			break; // ### - error?
		}
/* get module file name */
		file_name = arg_v[i];
		printf("    File name %-12s:", file_name);
/* convert DOS file name (relative or absolute; with or without drive letter)
to GRUB-style full path, with GRUB device name, forward slashes,
and lower-case file names */
		convert_name(&path, file_name);
/* open module file */
		f = open_or_die(file_name);
/* get length (size) of module file */
		lseek(f, 0, SEEK_END);
		len = tell(f);
		lseek(f, 0, SEEK_SET);
/* allocate memory for module */
		linear = alloc(len, g_pmode);
/* load entire module file into memory */
		read_linear(f, linear, len);
		printf(" %6lu bytes at linear address 0x%lX\n",
			len, linear);
/* close module file */
		close(f);
		f = -1;
/* init Multiboot module structure */
		g_mods[g_mboot.num_mods].start_adr = linear;
		g_mods[g_mboot.num_mods].end_adr = linear + len;
		g_mods[g_mboot.num_mods].cmd_line = to_linear(path);
		g_mboot.num_mods++;
	}
/* SET UP MULTIBOOT
lower and upper memory fields */
	for(i = 0; i < g_num_ranges; i++)
	{
		range = g_ranges + i;
/* ignore non-RAM ranges */
		if(range->type != 1)
			continue;
/* if range starts at address 0, store as conventional memory size */
		if(range->adr == 0)
			g_mboot.conv_mem = range->size;
/* if range starts at address 1 meg, store as extended memory size */
		if(range->adr == 0x100000uL)
			g_mboot.ext_mem = range->size;
	}
/* convert to K */
	g_mboot.conv_mem /= 1024;
	g_mboot.ext_mem /= 1024;
	g_mboot.flags |= MBF_MEMORY;
/* root device */
	_dos_getdrive(&i);
	if(get_drive_num(&drive, &part, (i - 1) + 'A') == 0)
	{
		g_mboot.root_dev = 0xFFFF0000L | (part << 8) | drive;
		g_mboot.flags |= MBF_ROOTDEV;
	}
/* modules */
	if(g_mboot.num_mods != 0)
	{
		g_mboot.mods_adr = to_linear(&g_mods);
		g_mboot.flags |= MBF_MODS;
	}
/* BIOS memory map */
	for(i = 0; i < g_num_ranges; i++)
	{
		range = g_ranges + i;
		range->len = sizeof(mboot_range_t) - 4; /* GRUB voodoo */
	}
	g_mboot.map_len = g_num_ranges * sizeof(mboot_range_t);
	g_mboot.map_adr = to_linear(&g_ranges);
	g_mboot.flags |= MBF_MEM_MAP;
/* xxx - hard drive info */
//{
//    static mboot_drive_t hd0 = { 10/* 12 */, 0x80, 0, 1023, 255, 63 };
//
//    g_mboot.drives_len = sizeof(hd0);
//    g_mboot.drives_adr = to_linear(&hd0);
//    g_mboot.flags |= MBF_DRIVE_INFO;
//}
/* ROM config table
do INT 15h AH=C0h, then check for CY=0 and AH=0 */
	regs.R_AX = 0xC000;
	intr(0x15, &regs);
	if((regs.R_FLAGS & 0x0001) == 0 && regs.R_AX < 0x100)
	{
		g_mboot.config_table = to_linear(MK_FP(regs.R_ES, regs.R_BX));
		g_mboot.flags |= MBF_CFG_TABLE;
	}
/* loader name */
	g_mboot.loader_name = to_linear("MBLOAD-C 0.5");
	g_mboot.flags |= MBF_LOADER_NAME;
/* turn off floppy motor(s) */
	outportb(0x3F2, 0);
/* run kernel */
	printf("Press Esc to abort, any other key to boot\n");
	if(getch() == 27)
		return 0;
	enter_pmode();
	longjmp(g_oops, (int)"Could not enter protected mode");
	return 1; /* not reached */
}
