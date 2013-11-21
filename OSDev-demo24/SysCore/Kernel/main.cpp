/******************************************************************************
   main.cpp
		-Kernel main program

   modified\ Oct 10 2010
   arthor\ Mike
******************************************************************************/

/**
Chapter 23
----------

KNOWN BUGS:

-There is a bug found in get_cmd(). It seems to add 1 extra character to the buffer
 when the entered text is all caps. Works fine if its in lowercase

-In volOpenFile(), fsys.cpp: if input contains ':', an additional character, 0x2, is added
 before ':' on input of function. For example, a:main.cpp becomes:

 'a' '' ':' 'm' 'a' 'i' 'n' '.' 'c' 'p' 'p' ''

-. and .. in pathnames also dont seem to work. Need to look into

Currently a workaround is provided in the routine.
Several chapters may need to be updated, please be patient. :-)
**/

#include <bootinfo.h>
#include <hal.h>
#include <kybrd.h>
#include <string.h>
#include <flpydsk.h>
#include <fat12.h>
#include <stdio.h>

#include "DebugDisplay.h"
#include "exception.h"
#include "mmngr_phys.h"
#include "mmngr_virtual.h"
#include "task.h"

/**
*	Memory region
*/
struct memory_region {

	uint32_t	startLo;	//base address
	uint32_t	startHi;
	uint32_t	sizeLo;		//length (in bytes)
	uint32_t	sizeHi;
	uint32_t	type;
	uint32_t	acpi_3_0;
};

uint32_t kernelSize=0;

extern void enter_usermode ();
extern void install_tss (uint32_t idx, uint16_t kernelSS, uint16_t kernelESP);
extern void syscall_init ();

//! set access bit
#define I86_GDT_DESC_ACCESS			0x0001			//00000001

//! descriptor is readable and writable. default: read only
#define I86_GDT_DESC_READWRITE			0x0002			//00000010

//! set expansion direction bit
#define I86_GDT_DESC_EXPANSION			0x0004			//00000100

//! executable code segment. Default: data segment
#define I86_GDT_DESC_EXEC_CODE			0x0008			//00001000

//! set code or data descriptor. defult: system defined descriptor
#define I86_GDT_DESC_CODEDATA			0x0010			//00010000

//! set dpl bits
#define I86_GDT_DESC_DPL			0x0060			//01100000

//! set "in memory" bit
#define I86_GDT_DESC_MEMORY			0x0080			//10000000

/**	gdt descriptor grandularity bit flags	***/

//! masks out limitHi (High 4 bits of limit)
#define I86_GDT_GRAND_LIMITHI_MASK		0x0f			//00001111

//! set os defined bit
#define I86_GDT_GRAND_OS			0x10			//00010000

//! set if 32bit. default: 16 bit
#define I86_GDT_GRAND_32BIT			0x40			//01000000

//! 4k grandularity. default: none
#define I86_GDT_GRAND_4K			0x80			//10000000

/**
*	Initialization
*/
void init (multiboot_info* bootinfo) {

	//! initialize our vmm
//	vmmngr_initialize ();

	//! clear and init display
	DebugClrScr (0x13);
	DebugGotoXY (0,0);
	DebugSetColor (0x17);

	hal_initialize ();

	enable ();
	setvect (0,(void (__cdecl &)(void))divide_by_zero_fault);
	setvect (1,(void (__cdecl &)(void))single_step_trap);
	setvect (2,(void (__cdecl &)(void))nmi_trap);
	setvect (3,(void (__cdecl &)(void))breakpoint_trap);
	setvect (4,(void (__cdecl &)(void))overflow_trap);
	setvect (5,(void (__cdecl &)(void))bounds_check_fault);
	setvect (6,(void (__cdecl &)(void))invalid_opcode_fault);
	setvect (7,(void (__cdecl &)(void))no_device_fault);
	setvect (8,(void (__cdecl &)(void))double_fault_abort);
	setvect (10,(void (__cdecl &)(void))invalid_tss_fault);
	setvect (11,(void (__cdecl &)(void))no_segment_fault);
	setvect (12,(void (__cdecl &)(void))stack_fault);
	setvect (13,(void (__cdecl &)(void))general_protection_fault);
	setvect (14,(void (__cdecl &)(void))page_fault);
	setvect (16,(void (__cdecl &)(void))fpu_fault);
	setvect (17,(void (__cdecl &)(void))alignment_check_fault);
	setvect (18,(void (__cdecl &)(void))machine_check_abort);
	setvect (19,(void (__cdecl &)(void))simd_fpu_fault);

	pmmngr_init ((size_t) bootinfo->m_memorySize, 0xC0000000 + kernelSize*512);

	memory_region*	region = (memory_region*)0x1000;

	for (int i=0; i<10; ++i) {

		if (region[i].type>4)
			break;

		if (i>0 && region[i].startLo==0)
			break;

		pmmngr_init_region (region[i].startLo, region[i].sizeLo);
	}
	pmmngr_deinit_region (0x100000, kernelSize*512);
	/*
		kernel stack location
	*/
	pmmngr_deinit_region (0x0, 0x10000);

	//! initialize our vmm
	vmmngr_initialize ();

	//! install the keyboard to IR 33, uses IRQ 1
	kkybrd_install (33);

	//! set drive 0 as current drive
	flpydsk_set_working_drive (0);

	//! install floppy disk to IR 38, uses IRQ 6
	flpydsk_install (38);

	//! initialize FAT12 filesystem
	fsysFatInitialize ();

	//! initialize system calls
	syscall_init ();

	//! initialize TSS
	install_tss (5,0x10,0x9000);
}

//! sleeps a little bit. This uses the HALs get_tick_count() which in turn uses the PIT
void sleep (int ms) {

	static int ticks = ms + get_tick_count ();
	while (ticks > get_tick_count ())
		;
}

//! wait for key stroke
KEYCODE	getch () {

	KEYCODE key = KEY_UNKNOWN;

	//! wait for a keypress
	while (key==KEY_UNKNOWN)
		key = kkybrd_get_last_key ();

	//! discard last keypress (we handled it) and return
	kkybrd_discard_last_key ();
	return key;
}

//! command prompt
void cmd () {

	DebugPrintf ("\nCommand> ");
}

//! gets next command
void get_cmd (char* buf, int n) {

	KEYCODE key = KEY_UNKNOWN;
	bool	BufChar;

	//! get command string
	int i=0;
	while ( i < n ) {

		//! buffer the next char
		BufChar = true;

		//! grab next char
		key = getch ();

		//! end of command if enter is pressed
		if (key==KEY_RETURN)
			break;

		//! backspace
		if (key==KEY_BACKSPACE) {

			//! dont buffer this char
			BufChar = false;

			if (i > 0) {

				//! go back one char
				unsigned y, x;
				DebugGetXY (&x, &y);
				if (x>0)
					DebugGotoXY (--x, y);
				else {
					//! x is already 0, so go back one line
					y--;
					x = DebugGetHorz ();
				}

				//! erase the character from display
				DebugPutc (' ');
				DebugGotoXY (x, y);

				//! go back one char in cmd buf
				i--;
			}
		}

		//! only add the char if it is to be buffered
		if (BufChar) {

			//! convert key to an ascii char and put it in buffer
			char c = kkybrd_key_to_ascii (key);
			if (c != 0) { //insure its an ascii char

				DebugPutc (c);
				buf [i++] = c;
			}
		}

		//! wait for next key. You may need to adjust this to suite your needs
		sleep (10);
	}

	//! null terminate the string
	buf [i] = '\0';
}

//! read command
void cmd_read () {

	//! get pathname
	char path[32];
	DebugPrintf ("\n\rex: \"file.txt\", \"a:\\file.txt\", \"a:\\folder\\file.txt\"\n\rFilename> ");
	get_cmd (path,32);

	//! open file
	FILE file = volOpenFile (path);

	//! test for invalid file
	if (file.flags == FS_INVALID) {

		DebugPrintf ("\n\rUnable to open file");
		return;
	}

	//! cant display directories
	if (( file.flags & FS_DIRECTORY ) == FS_DIRECTORY) {

		DebugPrintf ("\n\rUnable to display contents of directory.");
		return;
	}

	//! top line
	DebugPrintf ("\n\n\r-------[%s]-------\n\r", file.name);

	//! display file contents
	while (file.eof != 1) {

		//! read cluster
		unsigned char buf[512];
		volReadFile ( &file, buf, 512);

		//! display file
		for (int i=0; i<512; i++)
			DebugPutc (buf[i]);

		//! wait for input to continue if not EOF
		if (file.eof != 1) {
			DebugPrintf ("\n\r------[Press a key to continue]------");
			getch ();
			DebugPrintf ("\r"); //clear last line
		}
	}

	//! done :)
	DebugPrintf ("\n\n\r--------[EOF]--------");
}

void go_user () {

	int stack=0;
	_asm mov [stack], esp

	extern void tss_set_stack (uint16_t, uint16_t);
	tss_set_stack (0x10,(uint16_t) stack & 0xffff);

	enter_usermode();

	char testStr[]="\n\rWe are inside of your computer...";

	//! call OS-print message
	_asm xor eax, eax
	_asm lea ebx, [testStr]
	_asm int 0x80

	//! cant do CLI+HLT here, so loop instead
	while(1);
}

// proc (process) command
void cmd_proc () {

	int ret = 0;
	char name[32];

	DebugPrintf ("\n\rProgram file: ");
	get_cmd (name,32);

	ret = createProcess (name);
	if (ret==0)
		DebugPrintf ("\n\rError creating process");

	executeProcess ();
}

//! our simple command parser
bool run_cmd (char* cmd_buf) {

	if (strcmp (cmd_buf, "user") == 0) {
		go_user ();
	}

	//! exit command
	if (strcmp (cmd_buf, "exit") == 0) {
		return true;
	}

	//! clear screen
	else if (strcmp (cmd_buf, "cls") == 0) {
		DebugClrScr (0x17);
	}

	//! help
	else if (strcmp (cmd_buf, "help") == 0) {

		DebugPuts ("\nOS Development Series Process Management Demo");
		DebugPuts ("Supported commands:\n");
		DebugPuts (" - exit: quits and halts the system\n");
		DebugPuts (" - cls: clears the display\n");
		DebugPuts (" - help: displays this message\n");
		DebugPuts (" - read: reads a file\n");
		DebugPuts (" - reset: Resets and recalibrates floppy for reading\n");
		DebugPuts (" - proc: Run process");
	}

	//! read sector
	else if (strcmp (cmd_buf, "read") == 0) {
		cmd_read ();
	}

	//! run process
	else if (strcmp (cmd_buf, "proc") == 0) {
		cmd_proc();
	}

	//! invalid command
	else {
		DebugPrintf ("\nUnkown command");
	}

	return false;
}

void run () {

	//! command buffer
	char	cmd_buf [100];

	while (1) {

		//! display prompt
		cmd ();

		//! get command
		get_cmd (cmd_buf, 98);

		//! run command
		if (run_cmd (cmd_buf) == true)
			break;
	}
}

int _cdecl kmain (multiboot_info* bootinfo) {

	_asm	mov	word ptr [kernelSize], dx

	init (bootinfo);

	DebugGotoXY (0,0);
	DebugPuts ("OSDev Series Process Management Demo");
	DebugPuts ("\nType \"exit\" to quit, \"help\" for a list of commands\n");
	DebugPuts ("+-------------------------------------------------------+\n");

	run ();

	DebugPrintf ("\nExit command recieved; demo halted");
	_asm mov eax, 0xa0b0c0d0
	for (;;);
	return 0;
}
