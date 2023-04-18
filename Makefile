SRC		= $(shell pwd)
BIN		= ./Binaries
INCLUDE	= ./include
STD    	= c99
SYSROOT = root
KERNELDIR = $(SYSROOT)/orbit-kernel
LIBOSDIR = $(SYSROOT)/libraryOSes/relics-libOS
IVARS = -I$(SYSROOT)
KERNEL_IVARS = -I$(SYSROOT) -I$(KERNELDIR)
LIBOS_IVARS = -I$(SYSROOT) -I$(LIBOSDIR)

KERNEL_VERSION = 0.0.2

SOURCES_UTIL = $(KERNELDIR)/util/kio.c \
$(KERNELDIR)/util/memory.c \
$(KERNELDIR)/util/keyboard.c \
$(KERNELDIR)/util/port_functions.c \
$(KERNELDIR)/util/get_ascii_char.c \
$(KERNELDIR)/util/math.c \
$(KERNELDIR)/util/string.c \
$(KERNELDIR)/util/cpu.c \
$(KERNELDIR)/util/panic.c \
$(KERNELDIR)/util/idt.c \
$(KERNELDIR)/util/apploader.c \
$(KERNELDIR)/util/vga.c

SOURCES_LIBOS = $(LIBOSDIR)/apps/shell.c


SOURCES_32BIT =  \
$(KERNELDIR)/32bit/test_memory.c

OBJECTS_UTIL := $(SOURCES_UTIL:$(KERNELDIR)/util/%.c=$(BIN)/%.o)
OBJECTS_32BIT := $(SOURCES_32BIT:$(KERNELDIR)/32bit/%.c=$(BIN)/%.o)
OBJECTS_LIBOS := $(SOURCES_LIBOS:$(LIBOSDIR)/apps/%.c=$(BIN)/%.o)
OS_CFLAGS = -std=$(STD) -O0 -march=x86-64 \
			-pipe -nostdlib -ffreestanding -DVERSION=\"$(KERNEL_VERSION)\" $(CFLAGS)
LIBOS_CFLAGS = $(OS_CFLAGS) -D__userland  $(LIBOS_IVARS) $(EXTRA_LIBOS_CFLAGS)
OS_CFLAGS += \
			-fno-stack-protector \
			-mno-red-zone $(CFLAGS) $(KERNEL_IVARS) -mno-avx \
			-mno-sse -mno-sse2 -mno-3dnow -mcmodel=kernel --sysroot=$(SYSROOT) \
			-D__kernel -fno-PIC
OS_LDFLAGS = -nostdlib -z max-page-size=0x1000 $(LDFLAGS) $(EXTRA_KERNEL_CFLAGS)


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
	OS_CFLAGS += -D__X86_64__
ifeq ($(CC), clang)
	OS_CFLAGS += -target x86_64-none-elf
endif
endif



WFLAGS = -Wall -Wextra -Wpedantic \
         -Wshadow -Wvla -Wpointer-arith -Wwrite-strings -Wfloat-equal \
         -Wcast-align -Wcast-qual -Wbad-function-cast \
         -Wstrict-overflow=4 -Wunreachable-code -Wformat=2 \
         -Wundef  -Wno-unused-macros \
		 -Wno-reserved-macro-identifier -Wno-sign-conversion

all: clean build run

clean:
	-rm $(BIN)/*.o
	-rm $(BIN)/*.bin
build:
	$(MAKE) boot
	$(MAKE) kernel
	$(MAKE) $(OBJECTS_UTIL)
	$(MAKE) $(OBJECTS_32BIT)
	$(MAKE) $(OBJECTS_LIBOS)
	$(MAKE) fpu_file
	$(MAKE) together

boot:
	as --64 $(KERNELDIR)/16bit/bootloader.asm -o $(BIN)/boot.o --mx86-used-note=no
	ld -o $(BIN)/boot.bin $(BIN)/boot.o --oformat binary -e boot -melf_x86_64 -ffreestanding -shared -Ttext 0x7c00
	as $(KERNELDIR)/16bit/kernel_entry.asm -o $(BIN)/kernel_entry.o
	as $(KERNELDIR)/util/isr.asm -o $(BIN)/isr.o
	head -c 10240K < /dev/zero > $(BIN)/zeroes.bin

kernel:
	$(CC) $(OS_CFLAGS) $(WFLAGS) $(WEXTRA) -c $(KERNELDIR)/kernel.c -o $(BIN)/kernel.o

$(OBJECTS_UTIL): $(BIN)/%.o : $(KERNELDIR)/util/%.c
	$(CC) $(OS_CFLAGS) $(WFLAGS) $(WEXTRA) -c $< -o $@


$(OBJECTS_32BIT): $(BIN)/%.o : $(KERNELDIR)/32bit/%.c
	$(CC) $(OS_CFLAGS) $(WFLAGS) $(WEXTRA) -c $< -o $@

$(OBJECTS_LIBOS): $(BIN)/%.o : $(LIBOSDIR)/apps/%.c
	$(CC) $(LIBOS_CFLAGS) $(WFLAGS) $(WEXTRA) -c $< -o $@

fpu_file:
	$(CC) $(LIBOS_CFLAGS) $(WFLAGS) $(WEXTRA) -msse -c $(KERNELDIR)/util/cpu.c -o $(BIN)/cpu.o


together:
	$(LD) $(OS_LDFLAGS) -o $(BIN)/full_kernel.bin -Ttext 0x1000 \
	$(BIN)/kernel_entry.o $(BIN)/kernel.o $(BIN)/isr.o \
		$(OBJECTS_UTIL) $(OBJECTS_32BIT) $(OBJECTS_LIBOS) --oformat binary
	@cat $(BIN)/boot.bin $(BIN)/full_kernel.bin $(BIN)/zeroes.bin > $(BIN)/OS.bin


run:
	#qemu-system-x86_64 -monitor stdio -d int -no-shutdown -no-reboot -drive format=raw,file="$(BIN)/OS.bin",index=0,if=floppy,  -m 4G
	qemu-system-x86_64 -monitor stdio -drive format=raw,file="$(BIN)/OS.bin",index=0,if=floppy,  -m 256M

