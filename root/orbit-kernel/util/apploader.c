#include <apploader.h>
#include <kio.h>
#include <inttypes.h>
#include <string.h>

#define MEMORY_SIZE (32768 * 1024) - 0x7c00
/* Address will be the address of
 * a local variable from the kernel so that
 * we know the return address to make the jump.
 * The appcode is just a magic number that
 * indicates the pattern we search memory for. */
__attribute__((noreturn)) void
load_app(char *looking_for, long *address)
{
	/* Commented out because "app" loading is a
     * work-in-progress and I'd rather save
     * peoples' eyes from burning due to compiler
     * warnings */
#if 0
    char memory[14] = {0};
    looking_for = "LOAD-APP-0908";

    *(memory) = *(char *)0x7c00;
    for (int i = 0; i < MEMORY_SIZE; i++) {
        *(memory) = *(char *)(0x7c00 + i++);
        *(memory+1) = *(char *)(0x7c00 + i++);
        *(memory+2) = *(char *)(0x7c00 + i++);
        *(memory+3) = *(char *)(0x7c00 + i++);
        *(memory+4) = *(char *)(0x7c00 + i++);
        *(memory+5) = *(char *)(0x7c00 + i++);
        *(memory+6) = *(char *)(0x7c00 + i++);
        *(memory+7) = *(char *)(0x7c00 + i++);
        *(memory+8) = *(char *)(0x7c00 + i++);
        *(memory+9) = *(char *)(0x7c00 + i++);
        *(memory+10) = *(char *)(0x7c00 + i++);
        *(memory+11) = *(char *)(0x7c00 + i++);
        *(memory+12) = *(char *)(0x7c00 + i);
        if (strncmp(memory, looking_for, 13) == 0) break;
    }
    printk("WE FOUND SHELL!");
    /* return from function */
    __asm__ __volatile__("jmp *%0\t\n" : : "a" (&address));
#endif
}
