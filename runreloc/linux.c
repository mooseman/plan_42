/*****************************************************************************
LINUX SUPPORT ROUTINES

EXPORTS
int kbhit(void);
int getch(void);
void delay(unsigned milliseconds);
void sys_putch(unsigned c);
*****************************************************************************/
#include <sys/time.h> /* struct timeval, select() */
/* ICANON, ECHO, TCSANOW, struct termios */
#include <termios.h> /* tcgetattr(), tcsetattr() */
#include <stdlib.h> /* atexit(), exit() */
#include <unistd.h> /* read() */
#include <stdio.h> /* printf() */

static char g_init;
static struct termios g_old_kbd_mode;
/*****************************************************************************
*****************************************************************************/
static void cooked(void)
{
	tcsetattr(0, TCSANOW, &g_old_kbd_mode);
}
/*****************************************************************************
*****************************************************************************/
static void raw(void)
{
	struct termios new_kbd_mode;

/* put keyboard (stdin, actually) in raw, unbuffered mode */
	if(g_init)
		return;
	tcgetattr(0, &g_old_kbd_mode);
	memcpy(&new_kbd_mode, &g_old_kbd_mode, sizeof(struct termios));
	new_kbd_mode.c_lflag &= ~(ICANON | ECHO);
	new_kbd_mode.c_cc[VTIME] = 0;
	new_kbd_mode.c_cc[VMIN] = 1;
	tcsetattr(0, TCSANOW, &new_kbd_mode);
/* when we exit, go back to normal, "cooked" mode */
	atexit(cooked);

	g_init = 1;
}
/*****************************************************************************
*****************************************************************************/
int kbhit(void)
{
	struct timeval timeout;
	fd_set read_handles;
	int status;

	raw();
/* check stdin (fd 0) for activity */
	FD_ZERO(&read_handles);
	FD_SET(0, &read_handles);
	timeout.tv_sec = timeout.tv_usec = 0;
	status = select(1, &read_handles, NULL, NULL, &timeout);
	if(status < 0)
	{
		printf("select() failed in kbhit()\n");
		exit(1);
	}
	return status;
}
/*****************************************************************************
*****************************************************************************/
int getch(void)
{
	unsigned char temp;

	raw();
/* stdin = fd 0 */
	if(read(0, &temp, 1) != 1)
		return 0;
	return temp;
}
/*****************************************************************************
*****************************************************************************/
void delay(unsigned milliseconds)
{
	struct timeval timeout;

	timeout.tv_sec = 0;
	timeout.tv_usec = 1000 * milliseconds;
	(void)select(0, NULL, NULL, NULL, &timeout);
}
/*****************************************************************************
*****************************************************************************/
void sys_putch(unsigned c)
{
	static char init;
/**/
	if(!init)
	{
		setbuf(stdout, NULL);
		init = 0;
	}
	putchar(c);
}
/*****************************************************************************
*****************************************************************************/
void putch_help(unsigned c)
{
	static char init;
/**/
	if(!init)
	{
		setbuf(stdout, NULL);
		init = 1;
	}
	putchar(c);
}
