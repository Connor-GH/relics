.global idt_reload
.intel_syntax noprefix

idt_reload:
    push rbp
    mov rbp, rsp
    pushfq
    cli
    lidt [rdi] # first arg
    popfq
    pop rbp
    ret
