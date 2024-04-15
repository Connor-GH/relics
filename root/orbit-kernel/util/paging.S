.intel_syntax noprefix
.section .text
.global load_page_dir
.code64
load_page_dir:
push rbp
mov rbp, rsp
# mov rax, [rsp + 8] # rsp + 8?
mov rax, [rdi]
mov cr3, rax
mov rsp, rbp
pop rbp
ret

.global enable_paging
enable_paging:
push rbp
mov rbp, rsp
# enable paging
mov rax, cr0
bts rax, 31
mov cr0, rax

mov rsp, rbp
pop rbp
ret
