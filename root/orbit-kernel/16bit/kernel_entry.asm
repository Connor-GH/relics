.section .text
.intel_syntax noprefix
.code64
//.org 0x1000
.extern init_kernel
call init_kernel
hlt
