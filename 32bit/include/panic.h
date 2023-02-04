#ifndef KERNEL_PANIC_H
#define KERNEL_PANIC_H

void panic(int i);

#define CPU_ISSUE 0
#define MEMORY_ISSUE 1
#define GRAPHICS_ISSUE 2
#define STORAGE_ISSUE 3
#define GENERIC_ISSUE -1
#endif
