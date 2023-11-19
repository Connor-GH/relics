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
SOURCES_REALMODE = $(wildcard $(KERNELDIR)/16bit/*.c)
SOURCES_LIBOS = $(LIBOSDIR)/apps/shell.c


SOURCES_32BIT =  \
$(KERNELDIR)/32bit/test_memory.c

OBJECTS_UTIL := $(SOURCES_UTIL:$(KERNELDIR)/util/%.c=$(BIN)/%.o)
OBJECTS_REALMODE := $(SOURCES_REALMODE:$(KERNELDIR)/16bit/%.c=$(BIN)/%.o)
OBJECTS_32BIT := $(SOURCES_32BIT:$(KERNELDIR)/32bit/%.c=$(BIN)/%.o)
OBJECTS_LIBOS := $(SOURCES_LIBOS:$(LIBOSDIR)/apps/%.c=$(BIN)/%.o)
OS_CFLAGS = -std=$(STD) -O0 -march=x86-64 \
			-pipe -nostdlib -ffreestanding -DVERSION=\"$(KERNEL_VERSION)\" $(CFLAGS)
REALMODE_CFLAGS = $(OS_CFLAGS)
LIBOS_CFLAGS = $(OS_CFLAGS) -D__userland  $(LIBOS_IVARS) $(EXTRA_LIBOS_CFLAGS)
OS_CFLAGS += \
			-fno-stack-protector \
			-mno-red-zone $(KERNEL_IVARS) -mno-avx \
			-mno-sse -mno-3dnow -mcmodel=kernel --sysroot=$(SYSROOT) \
			-D__kernel -fno-PIC -DKERNEL_LOG
OS_LDFLAGS = -nostdlib -z max-page-size=0x1000 $(LDFLAGS) $(EXTRA_KERNEL_CFLAGS)

ifeq ($(FEATURE_FLAGS),sse)
	OS_CFLAGS += -msse
endif
ifeq ($(FEATURE_FLAGS),avx)
	OS_CFLAGS += -msse -mavx
endif

ifeq ($(CC), clang)
	CC	= clang
	LD 	= ld.lld

	WEXTRA = -Weverything
	PREFIX=llvm-
	REALMODE_CFLAGS += -target i386-none-elf

endif
ifeq ($(CC), cc)
	CC	= clang
	LD 	= ld.lld
	WEXTRA = -Weverything
	REALMODE_CFLAGS += -target i386-none-elf

endif
ifeq ($(CC), gcc)
	CC = gcc
	LD = ld
	# Hack to get ld to link the OS in a decent size.
	# Without it, the output executable is 14MB.
	OS_LDFLAGS += -shared -no-pie
	REALMODE_CFLAGS += -mcmodel=32 -Wl,-b,elf32-i386
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



ifeq ($(QEMU), debug)
	QEMU_RUN = qemu-system-x86_64 -monitor stdio -d int -no-shutdown -no-reboot -smp 4 -drive format=raw,file="$(BIN)/OS.bin",index=0,if=floppy,  -m 4G
else
	QEMU_RUN = qemu-system-x86_64 -monitor stdio -drive format=raw,file="$(BIN)/OS.bin",index=0,if=floppy,  -m 4G
endif

WFLAGS = -Wall -Wextra -Wpedantic \
         -Wshadow -Wvla -Wpointer-arith -Wwrite-strings -Wfloat-equal \
         -Wcast-align -Wcast-qual -Wbad-function-cast \
         -Wstrict-overflow=4 -Wunreachable-code -Wformat=2 \
         -Wundef  -Wno-unused-macros \
		 -Wno-reserved-macro-identifier -Wno-sign-conversion

all: clean build run

16BIT_CFLAGS = -m16
REALMODE_CFLAGS +=  $(16BIT_CFLAGS)
clean:
	-rm $(BIN)/*.o
	-rm $(BIN)/*.bin
build:
	$(MAKE) boot
	$(MAKE) kernel
	$(MAKE) $(OBJECTS_UTIL)
	$(MAKE) $(OBJECTS_32BIT)
	$(MAKE) $(OBJECTS_REALMODE)
	$(MAKE) $(OBJECTS_LIBOS)
	$(MAKE) together

boot:
	as --64 $(KERNELDIR)/asm/bootloader.asm -o $(BIN)/boot.o --mx86-used-note=no
	as --64 $(KERNELDIR)/asm/kernel_entry.asm -o $(BIN)/kernel_entry.o
	as $(KERNELDIR)/util/isr.asm -o $(BIN)/isr.o
	as $(KERNELDIR)/util/idt.asm -o $(BIN)/idt-asm.o
	head -c 10240K < /dev/zero > $(BIN)/zeroes.bin

kernel:
	$(CC) $(OS_CFLAGS) $(WFLAGS) $(WEXTRA) -c $(KERNELDIR)/kernel.c -o $(BIN)/kernel.o

$(OBJECTS_UTIL): $(BIN)/%.o : $(KERNELDIR)/util/%.c
	$(CC) $(OS_CFLAGS) $(WFLAGS) $(WEXTRA) -c $< -o $@


$(OBJECTS_32BIT): $(BIN)/%.o : $(KERNELDIR)/32bit/%.c
	$(CC) $(OS_CFLAGS) $(WFLAGS) $(WEXTRA) -c $< -o $@

$(OBJECTS_REALMODE): $(BIN)/%.o : $(KERNELDIR)/16bit/%.c
	$(CC) $(REALMODE_CFLAGS) $(WFLAGS) $(WEXTRA) -c $< -o $@
	$(PREFIX)objcopy -O elf64-x86-64 -I elf32-x86-64 $@ $@

$(OBJECTS_LIBOS): $(BIN)/%.o : $(LIBOSDIR)/apps/%.c
	$(CC) $(LIBOS_CFLAGS) $(WFLAGS) $(WEXTRA) -c $< -o $@


together:
	ld -o $(BIN)/boot.bin $(BIN)/boot.o \
		--oformat binary -e start -melf_x86_64 -ffreestanding -shared -Ttext 0x7c00
	$(LD) $(OS_LDFLAGS) -o $(BIN)/full_kernel.bin -Ttext 0x1000 \
	$(BIN)/kernel_entry.o $(BIN)/kernel.o $(BIN)/isr.o $(BIN)/idt-asm.o \
		$(OBJECTS_UTIL) $(OBJECTS_32BIT) $(OBJECTS_LIBOS) $(OBJECTS_REALMODE) --oformat binary
	@cat $(BIN)/boot.bin $(BIN)/full_kernel.bin $(BIN)/zeroes.bin > $(BIN)/OS.bin

run:
	$(QEMU_RUN)

