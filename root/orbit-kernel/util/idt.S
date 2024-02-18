.global idt_reload
.intel_syntax noprefix
.code64
idt_reload:
    push rbp
    mov rbp, rsp
    pushfq
    lidt [rdi] # first arg
    popfq
    pop rbp
    ret
