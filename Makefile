PACKAGE_NAME_NOTSTRING = Relics
PACKAGE_NAME = "$(PACKAGE_NAME_NOTSTRING)"
PACKAGE_VERSION = 0.0.4
C_STD ?= c11
SRC		= $(shell pwd)
BIN		= ./bin
INCLUDE	= ./include
STD    	= c99
SYSROOT = root
KERNELDIR = $(SYSROOT)/orbit-kernel
LIBOSDIR = $(SYSROOT)/libraryOSes/relics-libOS
IVARS = -I$(SYSROOT)
KERNEL_IVARS = -I$(SYSROOT) -I$(KERNELDIR)
LIBOS_IVARS = -I$(SYSROOT) -I$(LIBOSDIR)

KERNEL_VERSION = 0.0.3

SOURCES_UTIL = $(wildcard $(KERNELDIR)/util/*.c)
SOURCES_LIBOS = $(LIBOSDIR)/apps/shell.c

# D stuff
D_SOURCES = $(wildcard $(KERNELDIR)/util/d/*.d)

SOURCES_32BIT =  \
$(KERNELDIR)/32bit/test_memory.c

OBJECTS_UTIL := $(SOURCES_UTIL:$(KERNELDIR)/util/%.c=$(BIN)/%.o)
OBJECTS_32BIT := $(SOURCES_32BIT:$(KERNELDIR)/32bit/%.c=$(BIN)/%.o)
OBJECTS_LIBOS := $(SOURCES_LIBOS:$(LIBOSDIR)/apps/%.c=$(BIN)/%.o)

D_OBJECTS := $(D_SOURCES:$(KERNELDIR)/util/d/%.d=$(BIN)/%.o)

ZERO_STACK_PROTECTION = \
						$(call cc-option,-fcf-protection=none) \
						-fno-strict-aliasing -fno-stack-protector \
						-fomit-frame-pointer -fno-PIC #-fno-pic -fno-PIC
COMMON_OS_CFLAGS = -std=$(STD) -O0  \
			-pipe -nostdlib -ffreestanding -DVERSION=\"$(KERNEL_VERSION)\" \
			$(KERNEL_IVARS) $(ZERO_STACK_PROTECTION)

ifneq ($(call cc-option, -mpreferred-stack-boundary=4),)
      stack_align4 := -mpreferred-stack-boundary=2
      stack_align8 := -mpreferred-stack-boundary=3
else ifneq ($(call cc-option, -mstack-alignment=16),)
      stack_align4 := -mstack-alignment=4
      stack_align8 := -mstack-alignment=8
 endif

ifeq ($(CC), cc)
	CC=clang
endif

LIBOS_CFLAGS = $(COMMON_OS_CFLAGS) -D__userland  $(LIBOS_IVARS) $(LIBOSCFLAGS) \
			   $(IVARS)
OS_CFLAGS = $(COMMON_OS_CFLAGS) -march=x86-64 \
			-mno-red-zone $(KERNEL_IVARS) -mno-avx \
			-mno-sse -mno-3dnow -mcmodel=kernel --sysroot=$(SYSROOT) \
			-D__kernel -DKERNEL_LOG $(KCFLAGS)
OS_LDFLAGS = -nostdlib -z max-page-size=0x1000 $(LDFLAGS) $(KLDFLAGS)

ifeq ($(FEATURE_FLAGS),)
FEATURE_FLAGS = none
endif
ifeq ($(FEATURE_FLAGS),sse)
	OS_CFLAGS += -msse
endif
ifeq ($(FEATURE_FLAGS),avx)
	OS_CFLAGS += -msse -mavx
endif
ifeq ($(FEATURE_FLAGS),none)

endif

ifeq ($(CC), clang)
	CC	= clang
	LD 	= ld.lld
	WEXTRA = -Weverything
	PREFIX=llvm-
endif
ifeq ($(CC), cc)
	CC	= clang
	LD 	= ld.lld
	WEXTRA = -Weverything
endif
ifeq ($(CC), gcc)
	CC = gcc
	LD = ld
	# Hack to get ld to link the OS in a decent size.
	# Without it, the output executable is 14MB.
	OS_LDFLAGS += -shared -no-pie
endif


ifeq ($(BITS), 32)
	OS_CFLAGS += -D__WORDSIZE__=32
	OS_CFLAGS += -D__i686__ -D__i386__
ifeq ($(CC), clang)
	 OS_CFLAGS += -target i686-none-elf
endif
else
	OS_CFLAGS += -D__WORDSIZE__=64
	LIBOS_CFLAGS += -D__WORDSIZE__=64
	OS_CFLAGS += -D__X86_64__
	LIBOS_CFLAGS += -D__X86_64__
ifeq ($(CC), clang)
	OS_CFLAGS += -target x86_64-none-elf
endif
endif



ifeq ($(QEMU), debug)
	QEMU_RUN = qemu-system-x86_64 -monitor stdio -d int -no-shutdown -no-reboot -smp 4 -drive format=raw,file="$(BIN)/OS.bin",index=0,if=floppy,  -m 4G
else
	QEMU_RUN = qemu-system-x86_64 -monitor stdio -drive format=raw,file="$(BIN)/OS.bin",index=0,if=floppy,  -m 4G
endif
QEMU_RUN += $(QEMU_FLAGS)

include cc_and_flags.mk
OS_CFLAGS += $(_CFLAGS)
LIBOS_CFLAGS += $(_CFLAGS)
OS_LDFLAGS += $(_LFLAGS)
D_IVARS = -I$(KERNELDIR)/util/d/runtime

all:
	$(MAKE) clean
	$(MAKE) build
	$(MAKE) run

clean:
	-rm $(BIN)/*.o
	-rm $(BIN)/*.bin
build:
	$(MAKE) boot
	$(MAKE) kernel
	$(MAKE) $(OBJECTS_UTIL)
	$(MAKE) $(D_OBJECTS)
	$(MAKE) $(OBJECTS_32BIT)
	$(MAKE) $(OBJECTS_LIBOS)
	$(MAKE) together

boot:
	as --64 $(KERNELDIR)/asm/bootloader.asm -o $(BIN)/boot.o --mx86-used-note=no
	as --64 $(KERNELDIR)/asm/kernel_entry.asm -o $(BIN)/kernel_entry.o --mx86-used-note=no
	as --64 $(KERNELDIR)/util/isr.asm -o $(BIN)/isr.o --mx86-used-note=no
	as --64 $(KERNELDIR)/util/idt.asm -o $(BIN)/idt-asm.o --mx86-used-note=no
	as --64 $(KERNELDIR)/util/gdt.S -o $(BIN)/gdt-asm.o --mx86-used-note=no
	head -c 10240K < /dev/zero > $(BIN)/zeroes.bin

kernel:
	$(CC) $(OS_CFLAGS) -c $(KERNELDIR)/kernel.c -o $(BIN)/kernel.o

$(OBJECTS_UTIL): $(BIN)/%.o : $(KERNELDIR)/util/%.c
	$(CC) $(OS_CFLAGS) -c $< -o $@

$(D_OBJECTS): $(BIN)%.o : $(KERNELDIR)/util/d/%.d
	$(DCC) $(_DFLAGS) $(D_IVARS) -c $< $(DCC_BASIC_O)$@


$(OBJECTS_32BIT): $(BIN)/%.o : $(KERNELDIR)/32bit/%.c
	$(CC) $(OS_CFLAGS) -c $< -o $@

$(OBJECTS_LIBOS): $(BIN)/%.o : $(LIBOSDIR)/apps/%.c
	$(CC) $(LIBOS_CFLAGS) -c $< -o $@


together:
	$(LD) -o $(BIN)/boot.bin $(BIN)/boot.o \
		--oformat binary -e _start -melf_x86_64 -Ttext 0x7c00
	$(LD) $(OS_LDFLAGS) -o $(BIN)/full_kernel.bin -Ttext 0x1000 \
		$(BIN)/kernel_entry.o \
		$(BIN)/kernel.o $(BIN)/isr.o $(BIN)/idt-asm.o \
		$(BIN)/gdt-asm.o $(OBJECTS_UTIL) $(OBJECTS_32BIT) \
		$(OBJECTS_LIBOS) $(D_OBJECTS) --oformat binary
	@cat $(BIN)/boot.bin $(BIN)/full_kernel.bin $(BIN)/zeroes.bin > $(BIN)/OS.bin

run:
	$(QEMU_RUN)

format:
	@find . -iname *.h -o -iname *.c | xargs clang-format -style=file:.clang-format -i
