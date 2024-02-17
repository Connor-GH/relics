#ifndef __KERNEL_PANIC_H
#define __KERNEL_PANIC_H

void
panic(int i);
void panic2(int, const char *);

#define CPU_ISSUE 0
#define MEMORY_ISSUE 1
#define GRAPHICS_ISSUE 2
#define STORAGE_ISSUE 3
#define GENERIC_ISSUE -1
#endif /* __KERNEL_PANIC_H */
