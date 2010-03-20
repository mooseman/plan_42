/* define additional section ".dll" containing both code and data,
just to make relocation more challenging

Nov 9, 2002 -- GCC 3.x bitches about code and data in the same section,
so do not define DLL if using this compiler.

http://gcc.gnu.org/cgi-bin/gnatsweb.pl?cmd=view%20audit-trail&database=gcc&pr=6697
"Not a bug..." -- yes, it IS a bug. */
#if __GNUC__<3
#define DLL 1
#endif
/*****************************************************************************
RELOCATABLE TETRIS
*****************************************************************************/
/* ensure entry point is start of .text section */
int real_main(void);

int main(void)
{
	return real_main();
}
/*****************************************************************************
*****************************************************************************/
#include <time.h> /* time_t */

/* IMPORTS
from RUN.C */
extern const char *g_sys_ver;

time_t sys_time(void);
void sys_exit(int err);
int sys_select(unsigned *timeout);
void sys_read(void *buf, unsigned len);

/* from LIBC.C */
int sys_rand(void);
void sys_srand(int new_seed);
int sys_printf(const char *fmt, ...);

#define	KEY_LFT	0x14B
#define	KEY_RT	0x14D
#define	KEY_DN	0x150

/* dimensions of playing area */
#define	SCN_WID		15
#define	SCN_HT		20
/* direction vectors */
#define	DIR_U		{ 0, -1 }
#define	DIR_D		{ 0, +1 }
#define	DIR_L		{ -1, 0 }
#define	DIR_R		{ +1, 0 }
#define	DIR_U2		{ 0, -2 }
#define	DIR_D2		{ 0, +2 }
#define	DIR_L2		{ -2, 0 }
#define	DIR_R2		{ +2, 0 }
/* ANSI colors */
#define	COLOR_BLACK	0
#define	COLOR_RED	1
#define	COLOR_GREEN	2
#define	COLOR_YELLOW	3
#define	COLOR_BLUE	4
#define	COLOR_MAGENTA	5
#define	COLOR_CYAN	6
#define	COLOR_WHITE	7

typedef struct
{
	int delta_x, delta_y;
} vector_t;

typedef struct
{
/* pointer to shape rotated +/- 90 degrees */
	unsigned plus_90, minus_90;
	unsigned color; /* shape color */
	vector_t dir[4]; /* drawing instructions */
} shape_t;

static shape_t g_shapes[] =
{
/* shape #0:			cube */
	{
		0, 0, COLOR_BLUE,
		{
			DIR_U, DIR_R, DIR_D, DIR_L
		}
	},
/* shapes #1 & #2:		bar */
	{
		2, 2, COLOR_GREEN,
		{
			DIR_L, DIR_R, DIR_R, DIR_R
		}
	},
	{
		1, 1, COLOR_GREEN,
		{
			DIR_U, DIR_D, DIR_D, DIR_D
		}
	},
/* shapes #3 & #4:		'Z' shape */
	{
		4, 4, COLOR_CYAN,
		{
			DIR_L, DIR_R, DIR_D, DIR_R
		}
	},
	{
		3, 3, COLOR_CYAN,
		{
			DIR_U, DIR_D, DIR_L, DIR_D
		}
	},
/* shapes #5 & #6:		'S' shape */
	{
		6, 6, COLOR_RED,
		{
			DIR_R, DIR_L, DIR_D, DIR_L
		}
	},
	{
		5, 5, COLOR_RED,
		{
			DIR_U, DIR_D, DIR_R, DIR_D
		}
	},
/* shapes #7, #8, #9, #10:	'J' shape */
	{
		8, 10, COLOR_MAGENTA,
		{
			DIR_R, DIR_L, DIR_L, DIR_U
		}
	},
	{
		9, 7, COLOR_MAGENTA,
		{
			DIR_U, DIR_D, DIR_D, DIR_L
		}
	},
	{
		10, 8, COLOR_MAGENTA,
		{
			DIR_L, DIR_R, DIR_R, DIR_D
		}
	},
	{
		7, 9, COLOR_MAGENTA,
		{
			DIR_D, DIR_U, DIR_U, DIR_R
		}
	},
/* shapes #11, #12, #13, #14:	'L' shape */
	{
		12, 14, COLOR_YELLOW,
		{
			DIR_R, DIR_L, DIR_L, DIR_D
		}
	},
	{
		13, 11, COLOR_YELLOW,
		{
			DIR_U, DIR_D, DIR_D, DIR_R
		}
	},
	{
		14, 12, COLOR_YELLOW,
		{
			DIR_L, DIR_R, DIR_R, DIR_U
		}
	},
	{
		11, 13, COLOR_YELLOW,
		{
			DIR_D, DIR_U, DIR_U, DIR_L
		}
	},
/* shapes #15, #16, #17, #18:	'T' shape */
	{
		16, 18, COLOR_WHITE,
		{
			DIR_U, DIR_D, DIR_L, DIR_R2
		}
	},
	{
		17, 15, COLOR_WHITE,
		{
			DIR_L, DIR_R, DIR_U, DIR_D2
		}
	},
	{
		18, 16, COLOR_WHITE,
		{
			DIR_D, DIR_U, DIR_R, DIR_L2
		}
	},
	{
		15, 17, COLOR_WHITE,
		{
			DIR_R, DIR_L, DIR_D, DIR_U2
		}
	}
};

static unsigned char g_screen[SCN_WID][SCN_HT];

/* move this variable to a different section of the file,
to test the relocation code */
#ifdef DLL
extern unsigned char g_dirty[] __attribute__((section (".dll")));
#endif
static unsigned char g_dirty[SCN_HT];
/*****************************************************************************
*****************************************************************************/
/* move this function to a different section of the file,
to test the relocation code */
#ifdef DLL
void draw_block(unsigned, unsigned, unsigned) __attribute__((section (".dll")));
#endif
void draw_block(unsigned x_pos, unsigned y_pos, unsigned color)
{
	if(x_pos >= SCN_WID)
		x_pos = SCN_WID - 1;
	if(y_pos >= SCN_HT)
		y_pos = SCN_HT - 1;
	color &= 7;

	g_screen[x_pos][y_pos] = color;
	g_dirty[y_pos] = 1;
}
/*****************************************************************************
*****************************************************************************/
static int detect_block_hit(unsigned x_pos, unsigned y_pos)
{
	return g_screen[x_pos][y_pos];
}
/*****************************************************************************
*****************************************************************************/
/* move this function to a different section of the file,
to test the relocation code */
#ifdef DLL
void draw_shape(unsigned x_pos, unsigned y_pos, unsigned which_shape) __attribute__((section (".dll")));
#endif
void draw_shape(unsigned x_pos, unsigned y_pos, unsigned which_shape)
{
	unsigned i;

	for(i = 0; i < 4; i++)
	{
		draw_block(x_pos, y_pos, g_shapes[which_shape].color);
		x_pos += g_shapes[which_shape].dir[i].delta_x;
		y_pos += g_shapes[which_shape].dir[i].delta_y;
	}
	draw_block(x_pos, y_pos, g_shapes[which_shape].color);
}
/*****************************************************************************
*****************************************************************************/
static void erase_shape(unsigned x_pos, unsigned y_pos, unsigned which_shape)
{
	unsigned i;

	for(i = 0; i < 4; i++)
	{
		draw_block(x_pos, y_pos, COLOR_BLACK);
		x_pos += g_shapes[which_shape].dir[i].delta_x;
		y_pos += g_shapes[which_shape].dir[i].delta_y;
	}
	draw_block(x_pos, y_pos, COLOR_BLACK);
}
/*****************************************************************************
*****************************************************************************/
static int detect_shape_hit(unsigned x_pos, unsigned y_pos,
		unsigned which_shape)
{
	unsigned i;

	for(i = 0; i < 4; i++)
	{
		if(detect_block_hit(x_pos, y_pos))
			return 1;
		x_pos += g_shapes[which_shape].dir[i].delta_x;
		y_pos += g_shapes[which_shape].dir[i].delta_y;
	}
	if(detect_block_hit(x_pos, y_pos))
		return 1;
	return 0;
}
/*****************************************************************************
*****************************************************************************/
static void init_screen(void)
{
	unsigned x_pos, y_pos;

	for(y_pos = 0; y_pos < SCN_HT; y_pos++)
	{
/* force entire screen to be redrawn */
		g_dirty[y_pos] = 1;
		for(x_pos = 1; x_pos < (SCN_WID - 1); x_pos++)
			g_screen[x_pos][y_pos] = 0;
/* draw vertical edges of playing field */
		g_screen[0][y_pos] = g_screen[SCN_WID - 1][y_pos] = COLOR_BLUE;
	}
/* draw horizontal edges of playing field */
	for(x_pos = 0; x_pos < SCN_WID; x_pos++)
		g_screen[x_pos][0] = g_screen[x_pos][SCN_HT - 1] = COLOR_BLUE;
}
/*****************************************************************************
*****************************************************************************/
static void refresh(void)
{
	unsigned x_pos, y_pos;

	for(y_pos = 0; y_pos < SCN_HT; y_pos++)
	{
		if(!g_dirty[y_pos])
			continue;
/* gotoxy(0, y_pos) */
		sys_printf("\x1B[%d;1H", y_pos + 1);
		for(x_pos = 0; x_pos < SCN_WID; x_pos++)
/* 0xDB is a solid rectangular block in the PC character set */
			sys_printf("\x1B[%dm\xDB\xDB", 30 + g_screen[x_pos][y_pos]);
		g_dirty[y_pos] = 0;
	}
/* reset foreground color to gray */
	sys_printf("\x1B[37m");
}
/*****************************************************************************
*****************************************************************************/
static unsigned collapse(void)
{
	unsigned char solid_row[SCN_HT];
	unsigned solid_rows;
	int row, col, temp;

/* determine which rows are solidly filled */
	solid_rows = 0;
	for(row = 1; row < SCN_HT - 1; row++)
	{
		temp = 0;
		for(col = 1; col < SCN_WID - 1; col++)
		{
			if(detect_block_hit(col, row))
				temp++;
		}
		if(temp == SCN_WID - 2)
		{
			solid_row[row] = 1;
			solid_rows++;
		}
		else
			solid_row[row] = 0;
	}
	if(solid_rows == 0)
		return 0;
/* collapse them */
	for(temp = row = SCN_HT - 2; row > 0; row--, temp--)
	{
/* find a solid row */
		while(solid_row[temp])
			temp--;
/* copy it */
		if(temp < 1)
		{
			for(col = 1; col < SCN_WID - 1; col++)
				g_screen[col][row] = COLOR_BLACK;
		}
		else
		{
			for(col = 1; col < SCN_WID - 1; col++)
				g_screen[col][row] = g_screen[col][temp];
		}
		g_dirty[row] = 1;
	}
	refresh();
	return solid_rows;
}
/*****************************************************************************
*****************************************************************************/
static unsigned get_key(void)
{
	unsigned char c;

	sys_read(&c, 1);
/* DOS getch() returns 0 for extended scancodes
Win32 getch() returns 0xE0 */
	if(c != 0 && c != 0xE0)
		return c;
	sys_read(&c, 1);
	return 0x100 | c;
}
/*****************************************************************************
*****************************************************************************/
static unsigned get_key_with_timeout(void)
{
	static unsigned timeout = 200;
/**/

	if(sys_select(&timeout) != 0)
	{
		timeout = 200;
		return 0;
	}
	return get_key();
}
/*****************************************************************************
for MinGW32
*****************************************************************************/
#ifdef __WIN32__
int __main(void) { return 0; }
/* Nov 9, 2002 - WTF is this? */
void _alloca(void) { }
#endif
/*****************************************************************************
*****************************************************************************/
/* move this variable to a different section of the file,
to test the relocation code */
#ifdef DLL
extern unsigned key __attribute__((section (".dll")));
#endif
static unsigned key;

#if defined(__WIN32__)
const char *g_tetris_ver = "Win32 PE COFF";
#elif defined(__DJGPP__)
const char *g_tetris_ver = "DJGPP COFF";
#else
const char *g_tetris_ver = "ELF";
#endif

int real_main(void)
{
	unsigned fell, new_shape, new_x, new_y;
	unsigned shape = 0, x = 0, y = 0, lines;

/* re-seed the random number generator */
	sys_srand((unsigned)sys_time());
/* banner screen */
	sys_printf("\x1B[40;37;0m"); /* normal white on black text */
	sys_printf("\x1B[2J""\x1B[1;%dH""TETRIS by Alexei Pazhitnov",
		SCN_WID * 2 + 2);
	sys_printf("\x1B[2;%dH""system: %s, game: %s", SCN_WID * 2 + 2,
		g_sys_ver, g_tetris_ver);
	sys_printf("\x1B[3;%dH""Software by Chris Giese", SCN_WID * 2 + 2);
	sys_printf("\x1B[5;%dH""'1' and '2' rotate shape", SCN_WID * 2 + 2);
	sys_printf("\x1B[6;%dH""Arrow keys move shape", SCN_WID * 2 + 2);
	sys_printf("\x1B[7;%dH""Esc or Q quits", SCN_WID * 2 + 2);
NEW:
	sys_printf("\x1B[10;%dH""Press any key to begin", SCN_WID * 2 + 2);
/* await key pressed */
	if(get_key() == 27)
		sys_exit(0);
/* erase banner */
	sys_printf("\x1B[9;%dH""                      ", SCN_WID * 2 + 2);
	sys_printf("\x1B[10;%dH""                      ", SCN_WID * 2 + 2);
	init_screen();
	lines = 0;
	goto FOO;

	while(1)
	{
		fell = 0;
		new_shape = shape;
		new_x = x;
		new_y = y;
		key = get_key_with_timeout();
		if(key == 0)
		{
			new_y++;
			fell = 1;
		}
		else
		{
			if(key == 'q' || key == 'Q' || key == 27)
				//break;
				goto FIN;
			if(key == '1')
				new_shape = g_shapes[shape].plus_90;
			else if(key == '2')
				new_shape = g_shapes[shape].minus_90;
			else if(key == KEY_LFT)
			{
				if(x > 0)
					new_x = x - 1;
			}
			else if(key == KEY_RT)
			{
				if(x < SCN_WID - 1)
					new_x = x + 1;
			}
/*			else if(key == KEY_UP)
			{
				if(y > 0)
					new_y = y - 1; 	cheat
			} */
			else if(key == KEY_DN)
			{
				if(y < SCN_HT - 1)
					new_y = y + 1;
			}
			fell = 0;
		}
/* if nothing has changed, skip the bottom half of this loop */
		if(new_x == x && new_y == y && new_shape == shape)
			continue;
/* otherwise, erase old shape from the old pos'n */
		erase_shape(x, y, shape);
/* hit anything? */
		if(detect_shape_hit(new_x, new_y, new_shape) == 0)
		{
/* no, update pos'n */
			x = new_x;
			y = new_y;
			shape = new_shape;
		}
/* yes -- did the piece hit something while falling on its own? */
		else if(fell)
		{
/* yes, draw it at the old pos'n... */
			draw_shape(x, y, shape);
/* ... and spawn new shape */
FOO:			y = 3;
			x = SCN_WID / 2;
			shape = sys_rand();
			shape %= 19;
/* debug to get rid of "invisible" (black) shapes */
sys_printf("\x1B[24;0H""shape=%d ", shape);

			lines += collapse();
			sys_printf("\x1B[9;%dH""Lines: %u   ",
				SCN_WID * 2 + 2, lines);
/* if newly spawned shape hits something, game over */
			if(detect_shape_hit(x, y, shape))
FIN:			{
				sys_printf("\x1B[9;%dH""\x1B[37;40;1m"
					"       GAME OVER""\x1B[0m",
					SCN_WID * 2 + 2);
				goto NEW;
			}
		}
/* hit something because of user movement/rotate OR no hit: just redraw it */
		draw_shape(x, y, shape);
		refresh();
	}
	return 0;
}
