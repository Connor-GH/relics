.code64
.intel_syntax noprefix
.altmacro
.macro _pushaq
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
.endm

.macro _popaq
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
.endm

.extern exception_handler_2

.macro isr_err_stub num
.global isr_stub_\num\()
isr_stub_\num\():
	push \num\()
    #call exception_handler
    jmp isr_common_stub
    #iretq
.endm
.macro isr_no_err_stub num
.global isr_stub_\num\()
isr_stub_\num\():
	push 0
    push \num\()
    #call exception_handler
    jmp isr_common_stub
    #iretq
.endm

.macro IRQ num1, num2
.global irq\num1\()
irq\num1\():
 	push 0
	push \num2\()
	#call exception_handler
	jmp isr_common_stub
	#iretq
.endm

/*.macro def_syscall num
isr_stub_\num\():
    push 0
    push \num\()
    jmp syscall_common
.endm*/


isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31 #
# isr_no_err_stub 128 /* syscall */


IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47
/*.set i,32
.rept 16
    isr_no_err_stub %i
    .set i, i+1
.endr
def_syscall 0xAC

syscall_common:
    pushfq
    call syscall_handler # fix
    popfq
    iretq

syscall_handler: ret

.align 4
error_common:
    pushfq
    cld
    call exception_handler
    popfq
    # return from irq
    iretq
*/


.global isr_stub_table

.macro insert_isr num
.section .data
    .quad isr_stub_\num
.endm
.macro insert_irq num
.section .data
    .quad irq\num
.endm

.section .data
isr_stub_table:
.set i,0
.rept 31
    insert_isr %i
    .set i, i+1
.endr


# IRQs
irq_table:
.set i, 0
.rept 15
    insert_irq %i
    .set i, i+1
.endr

.extern exception_handler
isr_common_stub:
    _pushaq
    mov rdi, rsp
    #cli
    call exception_handler
    _popaq
    add rsp, 8
    iretq
