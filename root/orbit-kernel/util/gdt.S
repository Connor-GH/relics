.global gdt_reload
.intel_syntax noprefix
.code64

.set KERNEL_CODE, (0x08)
.set KERNEL_DATA, (0x10)

# other GDT works as-is
gdt_reload:
push rbp
mov rbp, rsp
mov ax, KERNEL_DATA
pushfq
cli
lgdt [rdi]
popfq
mov ds, ax
mov es, ax
mov ss, ax
pop rbp
pop rax
push KERNEL_CODE
push rax
xor rax, rax
retfq
/*gdt_reload:
lgdt [rdi]

mov eax, KERNEL_DATA # data segment
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax
mov rax, qword ptr .leave_gdt

push KERNEL_CODE # 64-bit code segment offset
push rax
retfq
.leave_gdt:
ret*/

