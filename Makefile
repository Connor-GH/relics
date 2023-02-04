SRC		= $(shell pwd)
BIN		= ./Binaries
INCLUDE	= ./include
STD    	= c99

SOURCES_UTIL = util/kio.c \
util/memory.c \
util/keyboard.c \
util/get_ascii_char.c \
util/math.c \
util/string.c
SOURCES_32BIT = 32bit/panic.c \
32bit/test_memory.c
OBJECTS_UTIL := $(SOURCES_UTIL:util/%.c=$(BIN)/%.o)
OBJECTS_32BIT := $(SOURCES_32BIT:32bit/%.c=$(BIN)/%.o)
OS_CFLAGS = -std=$(STD) -O3 -march=x86-64 \
			-pipe -nostdlib -ffreestanding \
			-fno-stack-protector -mno-red-zone $(CFLAGS) -mno-avx \
			-mno-sse
OS_LDFLAGS = -nostdlib -z max-page-size=0x1000 $(LDFLAGS)


ifneq (,$(filter $(CC),clang cc))
	CC	= clang
	LD 	= ld.lld

	WEXTRA = -Weverything
	OS_CFLAGS += -target x86_64-none-elf

endif



WFLAGS = -Wall -Wextra -Wpedantic \
         -Wshadow -Wvla -Wpointer-arith -Wwrite-strings -Wfloat-equal \
         -Wcast-align -Wcast-qual -Wbad-function-cast \
         -Wstrict-overflow=4 -Wunreachable-code -Wformat=2 \
         -Wundef  \
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
	$(MAKE) together

boot:
	@nasm 16bit/start_longmode_to_64bit.s -f bin -o $(BIN)/boot.bin
	@as 16bit/kernel_entry.asm -o $(BIN)/kernel_entry.o
	@nasm 16bit/zeroes.s 			-f bin -o $(BIN)/zeroes.bin

kernel:
	$(CC) $(OS_CFLAGS) $(WFLAGS) $(WEXTRA) -c -g "kernel.c" 			  -o"$(BIN)/kernel.o"

$(OBJECTS_UTIL): $(BIN)/%.o : util/%.c
	$(CC) $(OS_CFLAGS) $(WFLAGS) $(WEXTRA) -c -g $< -o $@


$(OBJECTS_32BIT): $(BIN)/%.o : 32bit/%.c
	$(CC) $(OS_CFLAGS) $(WFLAGS) $(WEXTRA) -c -g $< -o $@

together:
	$(LD) $(OS_LDFLAGS) -o $(BIN)/full_kernel.bin -Ttext 0x1000 \
		$(BIN)/kernel_entry.o $(BIN)/kernel.o \
		$(OBJECTS_UTIL) $(OBJECTS_32BIT) --oformat binary
	@cat $(BIN)/boot.bin $(BIN)/full_kernel.bin $(BIN)/zeroes.bin > $(BIN)/OS.bin


run:
	qemu-system-x86_64 -drive format=raw,file="$(BIN)/OS.bin",index=0,if=floppy,  -m 4G

