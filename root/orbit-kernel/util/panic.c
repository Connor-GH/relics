#include <stddef.h>
#include <kio.h>
#include <panic.h>


#define ERR_MSG (" unrecoverable error has occured.\n" \
        "The system will now go into an infinite loop.\n" \
        "Please restart the system through hardware.\n")

__attribute__((noreturn)) void panic(int i) {


    const char *issue;
    switch (i) {
    case CPU_ISSUE: issue = "processor-related"; break;
    case MEMORY_ISSUE: issue = "memory-related"; break;
    case GRAPHICS_ISSUE: issue = "graphics-related"; break;
    case STORAGE_ISSUE: issue = "storage-related"; break;
    default: /* fall though */
    case GENERIC_ISSUE: issue = "generic"; break;
    }

    printk("A %s unrecoverable error has occured.\n"
        "The system will now go into an infinite loop.\n"
        "Please restart the system through hardware.\n", issue);

    while (1) {}
}
