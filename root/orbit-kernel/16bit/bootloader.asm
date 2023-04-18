.section .text
.intel_syntax noprefix
.set KERNEL_LOCATION, 0x1000
.code16
start:
mov [BOOT_DISK], dl
xor ax, ax
mov es, ax
mov ds, ax
mov bp, 0x8000
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
mov dh, 35 /*800*/ /*35*/

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
#disable interrupts
cli

# enable A20 line, this enables bit number 20 in the address
in al,0x92
or al,2
out 0x92,al

# ds is uninitialized. lgdt uses ds as its segment so let's init it
xor ax,ax
mov ds,ax

lgdt [GDT_PTR]

mov eax, cr0
or eax, 0x1
mov cr0, eax
.set GDT_BOOT_CS_OFFSET, (GDT_BOOT_CS - GDT)
.set GDT_BOOT_DS_OFFSET, (GDT_BOOT_DS - GDT)

jmp GDT_BOOT_CS_OFFSET:protmode # jump using our new code segment from the gdt to set cs
s:
jmp s

.code32
protmode:
# we are now in 32-bit protected mode
mov ax, GDT_BOOT_DS_OFFSET
mov ds, ax
mov ss, ax
mov es, ax
mov fs, ax
mov gs, ax
mov esp, 0x90000 # tmp stack.
mov esp, ebp

call check_multiboot
call check_cpuid
call check_long_mode
jmp setup_paging_64bit

check_multiboot:
push    eax
mov     eax, 0x36d76289 # TODO: use multiboot bootloader
cmp     eax, 0x36d76289
jne     .no_multiboot
pop     eax
ret

.no_multiboot:
hlt

check_cpuid:
pushfd
pop     eax
mov     ecx, eax
xor     eax, 1 << 21
push    eax
popfd
pushfd
pop     eax
push    ecx
popfd
cmp     eax, ecx
je      .no_cpuid
ret

.no_cpuid:
hlt


check_long_mode:
mov     eax, 0x80000000
cpuid
cmp     eax, 0x80000001
jb      .no_long_mode

mov     eax, 0x80000001
cpuid
test    edx, 1 << 29
jz      .no_long_mode
ret


.no_long_mode:
hlt


# switching to 64-bit long mode
setup_paging_64bit:
mov edi, 0xb8000
mov ax, 0b01011100
# set PAE
mov eax,0xA0
mov cr4,eax
#
mov edi,0x100000 # first 1MiB is reserved for bios

# PD: 2 MiB pages
mov eax,0x83 # starts at 0x000000
xor edx,edx
mov [edi],eax
mov [edi+4],edx
add edi,8

mov ecx,511*2
xor eax,eax
rep stosd

# PDP: 1 GiB pages
mov eax,0x100003 # starts at 0x100000
mov [edi],eax
mov [edi+4],edx
add edi,8

mov ecx,511*2
xor eax,eax
rep stosd

# PML4: 512 GiB pages
mov eax,0x101003 # starts at 0x101000
mov [edi],eax
mov [edi+4],edx
add edi,8

mov ecx,511*2
xor eax,eax
rep stosd

# set PML4 pointer
mov eax,0x102000
mov cr3,eax

# set LME bit (long mode enable) in the IA32_EFER MSR
# MSRs are 64-bit wide and are written/read to/from eax:edx
mov ecx,0xC0000080 # this is the register number for EFER
mov eax,0x00000100 # LME bit set
xor edx,edx # other bits zero
wrmsr

# enable paging
mov eax,cr0
bts eax,31
mov cr0,eax

.set GDT_CS64_OFFSET, (GDT_CS64 - GDT)
jmp GDT_CS64_OFFSET:long_mode # as with the protected mode switch, we jmp using the new code segm

.code64
long_mode:
mov rax, GDT_CS64_OFFSET
jmp KERNEL_LOCATION
hlt

hang:
pause
jmp hang

BOOT_DISK: .byte 0
.align 4
GDT_PTR:
.word GDT_END-GDT-1
.long GDT # offset

.align 16
GDT:
GDT_NULL: .quad 0 # required on some platforms, disallow use of segment 0
GDT_BOOT_DS: .quad 0x00CF92000000FFFF
GDT_BOOT_CS: .quad 0x00CF9A000000FFFF # same as DS but with executable set in access byte
GDT_CS64:    .quad 0x00209A0000000000 # same as above but 64-bit
GDT_END:

.fill 510 - (. - start), 1, 0 # pad to 512 bytes
.word 0xAA55
