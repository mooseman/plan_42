/*****************************************************************************
ANSI/VT CONSOLE EMULATION

EXPORTS
void sys_putch(unsigned c);
*****************************************************************************/
#include <ctype.h> /* isdigit() */

/* IMPORTS
from C library or from WINDOWS.C */
void textattr(int newattr);
void clrscr(void);
void gotoxy(int x, int y);
int putch(int ch);

typedef struct _console
{
	unsigned attrib, esc, esc1, esc2, esc3, csr_x, csr_y;
} console_t;

static console_t g_con =
{
	0x1F,	/* bright white on blue */
	0, 0, 0, 0, 0, 0
};
static unsigned g_vc_width = 80, g_vc_height = 25;
/*****************************************************************************
*****************************************************************************/
static void set_attrib(console_t *con, unsigned att)
{
	static const unsigned char ansi_to_vga[] =
	{
		0, 4, 2, 6, 1, 5, 3, 7
	};
/**/
	unsigned new_att;

	new_att = con->attrib;
/* bold on/off */
	if(att == 0)
		new_att &= ~0x08;
	else if(att == 1)
		new_att |= 0x08;
/* set foreground color */
	else if(att >= 30 && att <= 37)
	{
		att = ansi_to_vga[att - 30];
		new_att = (new_att & ~0x07) | att;
	}
/* set background color */
	else if(att >= 40 && att <= 47)
	{
		att = ansi_to_vga[att - 40] << 4;
		new_att = (new_att & ~0x70) | att;
	}
	con->attrib = new_att;
	textattr(new_att);
}
/*****************************************************************************
*****************************************************************************/
void sys_putch(unsigned c)
{
	console_t *con = &g_con;

/* state machine to handle the escape sequences
ESC */
	if(con->esc == 1)
	{
		if(c == '[')
		{
			con->esc++;
			con->esc1 = 0;
			return;
		}
		/* else fall-through: zero esc and print c */
	}
/* ESC[ */
	else if(con->esc == 2)
	{
		if(isdigit(c))
		{
			con->esc1 = con->esc1 * 10 + c - '0';
			return;
		}
		else if(c == ';')
		{
			con->esc++;
			con->esc2 = 0;
			return;
		}
/* ESC[2J -- clear screen */
		else if(c == 'J')
		{
			if(con->esc1 == 2)
			{
				clrscr();
				con->csr_x = con->csr_y = 0;
			}
		}
/* ESC[num1m -- set attribute num1 */
		else if(c == 'm')
			set_attrib(con, con->esc1);
		con->esc = 0;	/* anything else with one numeric arg */
		return;
	}
/* ESC[num1; */
	else if(con->esc == 3)
	{
		if(isdigit(c))
		{
			con->esc2 = con->esc2 * 10 + c - '0';
			return;
		}
		else if(c == ';')
		{
			con->esc++;	/* ESC[num1;num2; */
			con->esc3 = 0;
			return;
		}
/* ESC[num1;num2H -- move cursor to num1,num2 */
		else if(c == 'H')
		{
			if(con->esc2 < g_vc_width)
				con->csr_x = con->esc2;
			if(con->esc1 < g_vc_height)
				con->csr_y = con->esc1;
			gotoxy(con->csr_x, con->csr_y);
		}
/* ESC[num1;num2m -- set attributes num1,num2 */
		else if(c == 'm')
		{
			set_attrib(con, con->esc1);
			set_attrib(con, con->esc2);
		}
		con->esc = 0;
		return;
	}
/* ESC[num1;num2;num3 */
	else if(con->esc == 4)
	{
		if(isdigit(c))
		{
			con->esc3 = con->esc3 * 10 + c - '0';
			return;
		}
/* ESC[num1;num2;num3m -- set attributes num1,num2,num3 */
		else if(c == 'm')
		{
			set_attrib(con, con->esc1);
			set_attrib(con, con->esc2);
			set_attrib(con, con->esc3);
		}
		con->esc = 0;
		return;
	}
	con->esc = 0;

/* escape character */
	if(c == 0x1B)
	{
		con->esc = 1;
		return;
	}
/* backspace */
	if(c == 0x08)
	{
		if(con->csr_x != 0)
			con->csr_x--;
	}
/* tab */
	else if(c == 0x09)
		con->csr_x = (con->csr_x + 8) & ~(8 - 1);
/* carriage return */
	else if(c == '\r')	/* 0x0D */
		con->csr_x = 0;
/* line feed */
//	else if(c == '\n')	/* 0x0A */
//		con->csr_y++;
/* CR/LF */
	else if(c == '\n')	/* ### - 0x0A again */
	{
		con->csr_x = 0;
		con->csr_y++;
	}
/* printable ASCII */
	else if(c >= ' ')
	{
		gotoxy(con->csr_x + 1, con->csr_y + 1);
		putch(c);
		con->csr_x++;
	}
	if(con->csr_x >= g_vc_width)
	{
		con->csr_x = 0;
		con->csr_y++;
	}
	if(con->csr_y >= g_vc_height)
		con->csr_y = g_vc_height - 1;
	gotoxy(con->csr_x + 1, con->csr_y + 1);
}
