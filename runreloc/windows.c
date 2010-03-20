/*****************************************************************************
WINDOWS SUPPORT ROUTINES

EXPORTS
void textattr(unsigned newattr);
void clrscr(void);
void gotoxy(unsigned x, unsigned y);
void delay(unsigned milliseconds);
*****************************************************************************/
#include <windows.h>
/*****************************************************************************
*****************************************************************************/
void textattr(unsigned newattr)
{
	HANDLE h;

	h = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(h, newattr);
}
/*****************************************************************************
*****************************************************************************/
void clrscr(void)
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	DWORD num_chars, count;
	COORD xy =
	{
		0, 0
	};
	HANDLE h;

	h = GetStdHandle(STD_OUTPUT_HANDLE);
	if(!GetConsoleScreenBufferInfo(h, &info))
		return;
	num_chars = info.dwSize.X * info.dwSize.Y;
/* fill the entire screen with blanks */
	if(!FillConsoleOutputCharacter(h, (TCHAR)' ',
		num_chars, xy, &count))
			return;
/* now set the buffer's attributes accordingly */
	if(!FillConsoleOutputAttribute(h, info.wAttributes,
		num_chars, xy, &count))
			return;
/* put the cursor at (0, 0) */
	SetConsoleCursorPosition(h, xy);
}
/*****************************************************************************
*****************************************************************************/
void gotoxy(unsigned x, unsigned y)
{
	COORD xy;
	HANDLE h;

	h = GetStdHandle(STD_OUTPUT_HANDLE);
	xy.X = x + 0;
	xy.Y = y;
	SetConsoleCursorPosition(h, xy);
}
/*****************************************************************************
*****************************************************************************/
void delay(unsigned milliseconds)
{
	Sleep(milliseconds);
}
