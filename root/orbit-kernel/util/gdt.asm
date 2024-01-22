.global gdt_reload
.intel_syntax noprefix

gdt_reload:
    push rbp
    mov rbp, rsp
    pushfq
    cli
    lgdt [rdi] # first arg
    popfq
    pop rbp
	call .gdt_reload_segments
    ret

.gdt_reload_segments:
push 0x8 # 64-bit code segment offset
lea rax, [.reload_code_segment]
push rax
retfq
.reload_code_segment:
mov ax, 0x10 # data segment
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax
ret
