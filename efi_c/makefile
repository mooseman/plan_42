# ---------------------------------------------------------------------------
#  Plan 42 build system  –  Linux + clang, x86‑64, public domain (Unlicense)
# ---------------------------------------------------------------------------

ARCH ?= x86_64
ARCH := $(strip $(ARCH))

# ---- arch guard -----------------------------------------------------------
ifeq ($(filter x86_64 amd64,$(ARCH)),)
$(error Only x86‑64 builds supported; invoke  make ARCH=x86_64)
endif

# ---- locate OVMF firmware -------------------------------------------------
OVMF ?= $(firstword \
        $(wildcard /usr/share/OVMF/OVMF_CODE_4M.fd) \
        $(wildcard /usr/share/ovmf/OVMF_4M.fd) \
        $(wildcard /usr/share/qemu/OVMF_4M.fd))

ifeq ($(OVMF),)
$(warning *** No OVMF firmware image found – pass OVMF=/path/to/OVMF.fd)
endif

# ---- toolchain triples ----------------------------------------------------
EFI_TRIPLE := $(ARCH)-pc-windows-msvc    # PE/COFF
ELF_TRIPLE := $(ARCH)-pc-elf             # freestanding ELF

EFICC       := clang -target $(EFI_TRIPLE)
EFI_LDFLAGS := -nostdlib -fuse-ld=lld-link \
               -Wl,-subsystem:efi_application \
               -Wl,-entry:efi_main

ELFCC       := clang -target $(ELF_TRIPLE)
ELFLD       := ld.lld

CC ?= clang                               # host compiler for helpers

# ---- directories ----------------------------------------------------------
PLAN_42_IMG_DIR := ../plan_42_img
BUILD_DIR   := ../build
QEMU_SCRIPT := ./qemu_$(ARCH).sh

# -------- create helper dirs (phony, silent) -------------------------------
.PHONY: $(PLAN_42_IMG_DIR) $(BUILD_DIR)
$(PLAN_42_IMG_DIR) $(BUILD_DIR):
	@mkdir -p $@

# ---- disk‑image generator -------------------------------------------------
IMG_SIZE_MIB ?= 64
DISK_IMG_PGM := $(PLAN_42_IMG_DIR)/mkp42img
PLAN_42_DISK     := $(BUILD_DIR)/plan-42.img

$(DISK_IMG_PGM): $(PLAN_42_IMG_DIR)/mkp42img.c | $(PLAN_42_IMG_DIR)
	$(CC) -O2 -static -s -o $@ $<

# ---- sources --------------------------------------------------------------
EFISRC  := efi.c
EFIOBJ  := $(EFISRC:%.c=%_$(ARCH).o)
EFI_APP := BOOTX64.EFI                    # PE32+ output

KERNEL_SRC := kernel.c
KERNEL      := kernel.elf                # ELF64 PIE

FONT := ter-132n.psf                     # bitmap font

# ---- compiler flags -------------------------------------------------------
COMMON_CFLAGS := -std=c17 -Wall -Wextra -Wpedantic -ffreestanding \
                 -mno-red-zone -fno-stack-protector -I include \
                 -DARCH=$(ARCH)

KERNEL_CFLAGS  := $(COMMON_CFLAGS) -fPIE
KERNEL_LDFLAGS := -nostdlib -pie -e kmain

DEPENDS := $(EFIOBJ:.o=.d) $(KERNEL_SRC:.c=.d)

# ---- targets --------------------------------------------------------------
all: $(PLAN_42_DISK)                          # default build

# ----- UEFI application (PE32+)
$(EFI_APP): $(EFIOBJ)
	$(EFICC) $(EFI_LDFLAGS) -o $@ $<

$(EFIOBJ): $(EFISRC)
	$(EFICC) $(COMMON_CFLAGS) -c -o $@ $<

# ----- Kernel (ELF64 PIE)
$(KERNEL): $(KERNEL_SRC)
	$(ELFCC) $(KERNEL_CFLAGS) $(KERNEL_LDFLAGS) -o $@ $<

# ----- Disk image (GPT + FAT32)
$(PLAN_42_DISK): $(DISK_IMG_PGM) $(EFI_APP) $(KERNEL) $(FONT) | $(BUILD_DIR)
	$(DISK_IMG_PGM) -o $@ -s $(IMG_SIZE_MIB) \
	    -ae /EFI/BOOT/$(EFI_APP) $(EFI_APP) \
	    -ad $(KERNEL)   kernel.elf \
	    -ad $(FONT)     $(FONT)
	@echo "Disk image created: $@"

# ----- Launch QEMU
.PHONY: run
run: $(PLAN_42_DISK)
	@if [ -z "$(OVMF)" ]; then \
	    echo "ERROR: OVMF firmware not found.  Pass OVMF=<file>"; exit 1; fi
	$(QEMU_SCRIPT) $(PLAN_42_DISK) $(OVMF)

# ----- clean
.PHONY: clean
clean:
	rm -f $(EFI_APP) $(EFIOBJ) $(KERNEL) *.o *.d *.elf *.EFI *.efi
	rm -f $(PLAN_42_DISK) $(DISK_IMG_PGM)

-include $(DEPENDS)



