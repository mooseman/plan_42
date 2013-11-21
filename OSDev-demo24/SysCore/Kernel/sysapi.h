
#include <hal.h>
#include "DebugDisplay.h"
#include "mmngr_virtual.h"
#include "task.h"

#define MAX_SYSCALL 2

static void* _syscalls[] = {

	DebugPrintf,
	TerminateProcess
};

_declspec(naked)
void syscall_dispatcher () {

	/* save index and set kernel data selector */
	static uint32_t idx=0;
	_asm {
		push eax
		mov eax, 0x10
		mov ds, ax
		pop eax
		mov [idx], eax
		pusha
	}

	//! bounds check
	if (idx>=MAX_SYSCALL) {
		_asm {
			/* restore registers and return */
			popa
			iretd
		}
	}

	//! get service
	static void* fnct = 0;
	fnct = _syscalls[idx];

	//! and call service
	_asm {
		/* restore registers */
		popa
		/* call service */
		push edi
		push esi
		push edx
		push ecx
		push ebx
		call fnct
		add esp, 20
		/* restore usermode data selector */
		push eax
		mov eax, 0x23
		mov ds, ax
		pop eax
		iretd
	}
}

//! from idt.h
#define I86_IDT_DESC_RING3		0x60

void syscall_init () {

	//! install interrupt handler!
	setvect (0x80, syscall_dispatcher, I86_IDT_DESC_RING3);
}

