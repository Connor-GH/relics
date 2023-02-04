.section .text
.extern init_kernel
call init_kernel
hlt
