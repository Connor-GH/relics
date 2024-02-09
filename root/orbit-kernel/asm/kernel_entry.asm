.section .text
.intel_syntax noprefix
.set KERNEL_LOCATION, 0x1000
.code16

mov esp, 0x90000
mov ebp, esp
.extern memory_map
call memory_map
cli
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
hlt
ret
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
mov ebp, esp # swapped s, b

cld
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

lgdt [.GDT64_PTR]

# enable paging
mov eax,cr0
bts eax,31
mov cr0,eax

.set GDT_CS64_OFFSET, (.GDT_CS64 - GDT64)
.set GDT_DS64_OFFSET, (.GDT_DS64 - GDT64)

jmp GDT_CS64_OFFSET:long_mode # as with the protected mode switch, we jmp using the new code segm

.code64
long_mode:
mov ax, GDT_DS64_OFFSET        # Set the A-register to the data descriptor.
mov ds, ax                    # Set the data segment to the A-register.
mov es, ax                    # Set the extra segment to the A-register.
mov fs, ax                    # Set the F-segment to the A-register.
mov gs, ax                    # Set the G-segment to the A-register.
mov ss, ax                    # Set the stack segment to the A-register.

.extern init_kernel
call init_kernel
hlt

hang:
pause
jmp hang


.section .data
.align 4
GDT_PTR:
.word GDT_END-GDT-1
.long GDT # offset

.align 16
GDT:
GDT_NULL: .quad 0 # required on some platforms, disallow use of segment 0
GDT_BOOT_CS: .quad 0x00CF9A000000FFFF # same as DS but with executable set in access byte
GDT_BOOT_DS: .quad 0x00CF92000000FFFF
GDT_END:
.set PRESENT,  1 << 7
.set NOT_SYS, 1 << 4
.set EXEC, 1 << 3
.set DC, 1 << 2
.set RW, 1 << 1
.set ACCESSED, 1 << 0

# Flags bits
.set GRAN_4K, 1 << 7
.set SZ_32, 1 << 6
.set LONG_MODE, 1 << 5

/*.code64
GDT64:
    .GDT64_NULL:
        .quad 0
    .GDT_CS64:
        .long 0xFFFF                                   # Limit & Base (low, bits 0-15)
        .byte 0                                        # Base (mid, bits 16-23)
        .byte PRESENT | NOT_SYS | EXEC | RW            # Access
        .byte GRAN_4K | LONG_MODE | 0xF                # Flags & Limit (high, bits 16-19)
        .byte 0 # Base (high, bits 24-31)
    .GDT_DS64:
        .long 0xFFFF                                   # Limit & Base (low, bits 0-15)
        .byte 0                                        # Base (mid, bits 16-23)
        .byte PRESENT | NOT_SYS | RW                   # Access
        .byte GRAN_4K | SZ_32 | 0xF                    # Flags & Limit (high, bits 16-19)
        .byte 0                                        # Base (high, bits 24-31)
    .GDT_64TSS:
        .long 0x00000068
        .long 0x00CF8900
    .GDT64_PTR:
        .word . - GDT64 - 1
        .quad GDT64*/
GDT64:
.GDT64_NULL: .quad 0
.GDT_CS64: .quad 0b0000000000100000100110000000000000000000000000000000000000000000 #  0x00209A0000000000 # same as above but 64-bit
.GDT_DS64: .quad 0b0000000000100000100100100000000000000000000000000000000000000000 # 158329674399744
GDT64_END:

.align 4
.GDT64_PTR:
.word GDT64_END-GDT64-1 # limit
.quad GDT64 # base

.section .text
.code64
.global ret_gdt_offset
ret_gdt_offset:
mov rax, GDT_CS64_OFFSET
ret

.global ret_gdt_cs
ret_gdt_cs:
mov rax, .GDT_CS64
ret

.global ret_gdt_ds
ret_gdt_ds:
mov rax, .GDT_DS64
ret

