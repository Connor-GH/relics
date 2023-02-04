#include "../util/include/kio.h"
#include "../util/include/memory.h"
#include "../util/include/string.h"
#include "include/test_memory.h"
#include "include/panic.h"

void
test_memory(void) {

    char *unsafe_buf;
    char *safe_buf;
    init_all_memory();
    init_vga(BRIGHT_GREEN, BLACK);
    reset_video_memory();
    print_string("Your memory has been initialized. You have ");
    print_int(all_mem_available_bytes());
    print_string(" bytes to your disposal.\n");

    print_string("Testing malloc()...\n");
    print_string("Creating memory-smashing buffer...");

    unsafe_buf = malloc(sizeof(char) * 33 * 1024 * 1024);
    if (unsafe_buf == NULL) {
        print_string("malloc() protections worked.\n");
    } else {
        print_string("malloc() protections failed.\n");
        panic(MEMORY_ISSUE);
        free(unsafe_buf);
    }
    print_string("Creating buffer with a safe amount of memory...");
    safe_buf = malloc(sizeof(char) * 1);
   if (safe_buf == NULL) {
        print_string("malloc() failed\n");
        panic(MEMORY_ISSUE);
    } else {
        print_string("malloc() worked.\n");
        free(unsafe_buf);
    }
}
