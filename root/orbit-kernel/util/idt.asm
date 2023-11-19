.global idt_reload
.intel_syntax noprefix

idt_reload:
    push rbp
    mov rbp, rsp
    pushfq
    cli
    lidt [rdi]
    popfq
    pop rbp
    ret
