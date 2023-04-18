BIN="./Binaries"
nasm "boot.s" -f bin -o "$BIN/boot.bin"                     # build ultrabasic boot binary
nasm "kernel_entry.s" -f elf -o "$BIN/kernel_entry.o"
nasm "zeroes.s" -f bin -o "$BIN/zeroes.bin"                 # zeroes to insure we have room

i686-elf-gcc -Wall -Wpedantic -Werror -ffreestanding -m32 -g -c "kernel.c" -o "$BIN/kernel.o"


i686-elf-ld -o "$BIN/full_kernel.bin" -Ttext 0x1000 \
    "$BIN/kernel_entry.o" "$BIN/kernel.o" -Wl,--oformat binary -L. -lgcc

cat "$BIN/boot.bin" "$BIN/full_kernel.bin" "$BIN/zeroes.bin"  > "$BIN/OS.bin"

qemu-system-x86_64 -drive format=raw,file="$BIN/OS.bin",index=0,if=floppy,  -m 128M
