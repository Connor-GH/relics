#include <stddef.h>
#include "../util/include/kio.h"
#include "include/panic.h"


#define ERR_MSG (" unrecoverable error has occured.\n" \
        "The system will now go into an infinite loop.\n" \
        "Please restart the system through hardware.\n")

__attribute__((noreturn)) void panic(int i) {


    print_string("A ");
    switch (i) {
    case CPU_ISSUE: print_string("processor-related"); break;
    case MEMORY_ISSUE: print_string("memory-related"); break;
    case GRAPHICS_ISSUE: print_string("graphics-related"); break;
    case STORAGE_ISSUE: print_string("storage-related"); break;
    default: /* fall though */
    case GENERIC_ISSUE: print_string("generic"); break;
    }

    print_string(ERR_MSG);

    while (1) {}
}
