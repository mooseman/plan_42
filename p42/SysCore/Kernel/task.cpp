//****************************************************************************
//**
//**    task.cpp
//**		-Task Manager
//**
//****************************************************************************
//============================================================================
//    IMPLEMENTATION HEADERS
//============================================================================

#include <string.h>
#include "fsys.h"
#include "image.h"
#include "mmngr_virtual.h"
#include "task.h"
#include "DebugDisplay.h"

//============================================================================
//    IMPLEMENTATION PRIVATE DEFINITIONS / ENUMERATIONS / SIMPLE TYPEDEFS
//============================================================================

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

//============================================================================
//    IMPLEMENTATION PRIVATE CLASS PROTOTYPES / EXTERNAL CLASS REFERENCES
//============================================================================
//============================================================================
//    IMPLEMENTATION PRIVATE STRUCTURES / UTILITY CLASSES
//============================================================================
//============================================================================
//    IMPLEMENTATION REQUIRED EXTERNAL REFERENCES (AVOID)
//============================================================================
//============================================================================
//    IMPLEMENTATION PRIVATE DATA
//============================================================================

#define PROC_INVALID_ID -1
static process _proc = {
	PROC_INVALID_ID,0,0,0,0
};

//============================================================================
//    INTERFACE DATA
//============================================================================
//============================================================================
//    IMPLEMENTATION PRIVATE FUNCTION PROTOTYPES
//============================================================================

//============================================================================
//    IMPLEMENTATION PRIVATE FUNCTIONS
//============================================================================

/**
* Return current process
* \ret Process
*/
process* getCurrentProcess() {
	return &_proc;
}

/**
* Map kernel space into address space
* \param addressSpace Page directory
*/
void mapKernelSpace (pdirectory* addressSpace) {
	uint32_t virtualAddr;
	uint32_t physAddr;
	/*
		default flags. Note USER bit not set to prevent user mode access
	*/
	int flags = I86_PTE_PRESENT|I86_PTE_WRITABLE;
	/*
		map kernel stack space (at idenitity mapped 0x8000-0x9fff)
	*/
	vmmngr_mapPhysicalAddress (addressSpace, 0x8000, 0x8000, flags);
	vmmngr_mapPhysicalAddress (addressSpace, 0x9000, 0x9000, flags);
	/*
		map kernel image (7 pages at physical 1MB, virtual 3GB)
	*/
	virtualAddr = 0xc0000000;
	physAddr    = 0x100000;
	for (uint32_t i=0; i<10; i++) {
		vmmngr_mapPhysicalAddress (addressSpace,
			virtualAddr+(i*PAGE_SIZE),
			physAddr+(i*PAGE_SIZE),
			flags);
	}
	/*
		map display memory for debug minidriver
		idenitity mapped 0xa0000-0xBF000.
		Note:
			A better alternative is to have a driver associated
			with the physical memory range map it. This should be automatic;
			through an IO manager or driver manager.
	*/
	virtualAddr = 0xa0000;
	physAddr = 0xa0000;
	for (uint32_t i=0; i<31; i++) {
		vmmngr_mapPhysicalAddress (addressSpace,
			virtualAddr+(i*PAGE_SIZE),
			physAddr+(i*PAGE_SIZE),
			flags);
	}

	/* map page directory itself into its address space */
	vmmngr_mapPhysicalAddress (addressSpace, (uint32_t) addressSpace,
			(uint32_t) addressSpace, I86_PTE_PRESENT|I86_PTE_WRITABLE);
}

/**
* Validate image
* \param image Base of image
* \ret Status code
*/
int validateImage (void* image) {
    IMAGE_DOS_HEADER* dosHeader = 0;
    IMAGE_NT_HEADERS* ntHeaders = 0;

    /* validate program file */
    dosHeader = (IMAGE_DOS_HEADER*) image;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
            return 0;
    }
    if (dosHeader->e_lfanew == 0) {
            return 0;
    }

    /* make sure header is valid */
    ntHeaders = (IMAGE_NT_HEADERS*)(dosHeader->e_lfanew + (uint32_t)image);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
            return 0;
    }

    /* only supporting for i386 archs */
    if (ntHeaders->FileHeader.Machine != IMAGE_FILE_MACHINE_I386) {
            return 0;
    }

    /* only support 32 bit executable images */
    if (! (ntHeaders->FileHeader.Characteristics &
            (IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_32BIT_MACHINE))) {
            return 0;
    }
    /*
            Note: 1st 4 MB remains idenitity mapped as kernel pages as it contains
            kernel stack and page directory. If you want to support loading below 1MB,
            make sure to move these into kernel land
    */
    if ( (ntHeaders->OptionalHeader.ImageBase < 0x400000)
            || (ntHeaders->OptionalHeader.ImageBase > 0x80000000)) {
            return 0;
    }

    /* only support 32 bit optional header format */
    if (ntHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
            return 0;
    }
	return 1;
}

//============================================================================
//    INTERFACE FUNCTIONS
//============================================================================

/**
* Create process
* \param appname Application file name
* \ret Status code
*/
int createProcess (char* appname) {

        IMAGE_DOS_HEADER* dosHeader = 0;
        IMAGE_NT_HEADERS* ntHeaders = 0;
        FILE file;
        pdirectory* addressSpace = 0;
        process* proc = 0;
        thread* mainThread = 0;
        unsigned char* memory = 0;
        unsigned char buf[512];
        uint32_t i = 0;

        /* open file */
        file = volOpenFile (appname);
        if (file.flags == FS_INVALID)
                return 0;
        if (( file.flags & FS_DIRECTORY ) == FS_DIRECTORY)
                return 0;

        /* read 512 bytes into buffer */
        volReadFile ( &file, buf, 512);
		if (! validateImage (buf)) {
			volCloseFile ( &file );
			return 0;
		}
        dosHeader = (IMAGE_DOS_HEADER*)buf;
        ntHeaders = (IMAGE_NT_HEADERS*)(dosHeader->e_lfanew + (uint32_t)buf);

        /* get process virtual address space */
//        addressSpace = vmmngr_createAddressSpace ();
		addressSpace = vmmngr_get_directory ();
		if (!addressSpace) {
                volCloseFile (&file);
                return 0;
        }
		/*
			map kernel space into process address space.
			Only needed if creating new address space
		*/
		//mapKernelSpace (addressSpace);

        /* create PCB */
        proc = getCurrentProcess();
        proc->id            = 1;
        proc->pageDirectory = addressSpace;
        proc->priority      = 1;
        proc->state         = PROCESS_STATE_ACTIVE;
        proc->threadCount   = 1;

		/* create thread descriptor */
        mainThread               = &proc->threads[0];
        mainThread->kernelStack  = 0;
        mainThread->parent       = proc;
        mainThread->priority     = 1;
        mainThread->state        = PROCESS_STATE_ACTIVE;
        mainThread->initialStack = 0;
        mainThread->stackLimit   = (void*) ((uint32_t) mainThread->initialStack + 4096);
		mainThread->imageBase    = ntHeaders->OptionalHeader.ImageBase;
		mainThread->imageSize    = ntHeaders->OptionalHeader.SizeOfImage;
        memset (&mainThread->frame, 0, sizeof (trapFrame));
        mainThread->frame.eip    = ntHeaders->OptionalHeader.AddressOfEntryPoint
                + ntHeaders->OptionalHeader.ImageBase;
        mainThread->frame.flags  = 0x200;

        /* copy our 512 block read above and rest of 4k block */
        memory = (unsigned char*)pmmngr_alloc_block();
        memset (memory, 0, 4096);
        memcpy (memory, buf, 512);

		/* load image into memory */
		for (i=1; i <= mainThread->imageSize/512; i++) {
                if (file.eof == 1)
                        break;
                volReadFile ( &file, memory+512*i, 512);
        }

        /* map page into address space */
        vmmngr_mapPhysicalAddress (proc->pageDirectory,
                ntHeaders->OptionalHeader.ImageBase,
                (uint32_t) memory,
                I86_PTE_PRESENT|I86_PTE_WRITABLE|I86_PTE_USER);

		/* load and map rest of image */
        i = 1;
        while (file.eof != 1) {
                /* allocate new frame */
                unsigned char* cur = (unsigned char*)pmmngr_alloc_block();
                /* read block */
                int curBlock = 0;
                for (curBlock = 0; curBlock < 8; curBlock++) {
                        if (file.eof == 1)
                                break;
                        volReadFile ( &file, cur+512*curBlock, 512);
                }
                /* map page into process address space */
                vmmngr_mapPhysicalAddress (proc->pageDirectory,
                        ntHeaders->OptionalHeader.ImageBase + i*4096,
                        (uint32_t) cur,
                        I86_PTE_PRESENT|I86_PTE_WRITABLE|I86_PTE_USER);
                i++;
        }

		/* Create userspace stack (process esp=0x100000) */
		void* stack =
			(void*) (ntHeaders->OptionalHeader.ImageBase
				+ ntHeaders->OptionalHeader.SizeOfImage + PAGE_SIZE);
		void* stackPhys = (void*) pmmngr_alloc_block ();

		/* map user process stack space */
		vmmngr_mapPhysicalAddress (addressSpace,
				(uint32_t) stack,
				(uint32_t) stackPhys,
				I86_PTE_PRESENT|I86_PTE_WRITABLE|I86_PTE_USER);

		/* final initialization */
		mainThread->initialStack = stack;
        mainThread->frame.esp    = (uint32_t)mainThread->initialStack;
        mainThread->frame.ebp    = mainThread->frame.esp;

		/* close file and return process ID */
		volCloseFile(&file);
        return proc->id;
}

/**
* Execute process
*/
void executeProcess () {
        process* proc = 0;
        int entryPoint = 0;
        unsigned int procStack = 0;

        /* get running process */
        proc = getCurrentProcess();
		if (proc->id==PROC_INVALID_ID)
			return;
        if (!proc->pageDirectory)
			return;

        /* get esp and eip of main thread */
        entryPoint = proc->threads[0].frame.eip;
        procStack  = proc->threads[0].frame.esp;

        /* switch to process address space */
        __asm cli
        pmmngr_load_PDBR ((physical_addr)proc->pageDirectory);

        /* execute process in user mode */
        __asm {
                mov     ax, 0x23        ; user mode data selector is 0x20 (GDT entry 3). Also sets RPL to 3
                mov     ds, ax
                mov     es, ax
                mov     fs, ax
                mov     gs, ax
				;
				; create stack frame
				;
				push   0x23				; SS, notice it uses same selector as above
				push   [procStack]		; stack
				push    0x200			; EFLAGS
				push    0x1b			; CS, user mode code selector is 0x18. With RPL 3 this is 0x1b
				push    [entryPoint]	; EIP
				iretd
        }
}

/* kernel command shell */
extern void run ();

/**
* TerminateProcess system call
*/
extern "C" {
void TerminateProcess () {
	process* cur = &_proc;
	if (cur->id==PROC_INVALID_ID)
		return;

	/* release threads */
	int i=0;
	thread* pThread = &cur->threads[i];

	/* get physical address of stack */
	void* stackFrame = vmmngr_getPhysicalAddress (cur->pageDirectory,
		(uint32_t) pThread->initialStack); 

	/* unmap and release stack memory */
	vmmngr_unmapPhysicalAddress (cur->pageDirectory, (uint32_t) pThread->initialStack);
	pmmngr_free_block (stackFrame);

	/* unmap and release image memory */
	for (uint32_t page = 0; page < pThread->imageSize/PAGE_SIZE; page++) {
		uint32_t phys = 0;
		uint32_t virt = 0;

		/* get virtual address of page */
		virt = pThread->imageBase + (page * PAGE_SIZE);

		/* get physical address of page */
		phys = (uint32_t) vmmngr_getPhysicalAddress (cur->pageDirectory, virt);

		/* unmap and release page */
		vmmngr_unmapPhysicalAddress (cur->pageDirectory, virt);
		pmmngr_free_block ((void*)phys);
	}

	/* restore kernel selectors */
	__asm {
		cli
		mov eax, 0x10
		mov ds, ax
		mov es, ax
		mov fs, ax
		mov gs, ax
		sti
	}

	/* return to kernel command shell */
	run ();

	DebugPrintf ("\nExit command recieved; demo halted");
	for (;;);
}
} // extern "C"

//============================================================================
//    INTERFACE CLASS BODIES
//============================================================================
//****************************************************************************
//**
//**    END[task.cpp]
//**
//****************************************************************************
