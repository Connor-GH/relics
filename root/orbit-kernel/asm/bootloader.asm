.section .text
.intel_syntax noprefix
.set KERNEL_LOCATION, 0x1000
.code16
start:
mov [BOOT_DISK], dl
xor ax, ax
mov es, ax
mov ds, ax
mov bp, 0xF000 # temp stack at 0x8000
mov sp, bp

mov bx, KERNEL_LOCATION

/* Keep up with this number.
 * it needs to correlate with
 * the size of OS.bin divided by 512;
 * it represents how many drive sectors to load.
 *
 * This is the temporary solution until a disk
 * filesystem gets written.
 */
mov dh, 53

mov ah, 0x02
mov al, dh
mov ch, 0x0
mov dh, 0x0
mov cl, 0x02
mov dl, [BOOT_DISK]
int 0x13

mov ah,0x00 # set mode
mov al,0x03 # text mode 80x25
int 0x10
/* as of right now,
* only two graphics modes are
* supported:
* VGA 300x200 256-color
* 640x480 16-color text mode
*/
#mov ax,0x13 # VGA 300x200 256 color
#int 0x10
#mov ax,0x12 # VGA 640x480 16 color
#int 0x10
# enable A20 line, this enables bit number 20 in the address
in al,0x92
or al,2
out 0x92,al


jmp KERNEL_LOCATION
hlt


BOOT_DISK: .byte 0
.fill 510 - (. - start), 1, 0 # pad to 512 bytes
.word 0xAA55
