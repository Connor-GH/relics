#include <kio.h>
#include <memory.h>
#include <orbit-kernel/string.h>
#include <32bit/include/test_memory.h>
#include <orbit-kernel/panic.h>

void
test_memory(void) {

    char *unsafe_buf;
    char *safe_buf;
    // init_all_memory(); // is fine
    init_vga(BRIGHT_GREEN, BLACK);
    reset_video_memory();

    printk("Your memory has been initialized. "
            "You have %ld bytes to your disposal.\n",
            all_mem_available_bytes());
	mem_into_pages();

    printk("Testing malloc()...\n");

	char *buf = malloc(sizeof("hello, world")+1);
	kernel_strncpy(buf, "hello, world", sizeof("hello, world")+1);
	printk("%s\n", buf);
	free(buf);

    printk("Creating memory-smashing buffer...");
    unsafe_buf = malloc(sizeof(char) * 33 * 1024 * 1024);
    if (unsafe_buf == NULL) {
        printk("malloc() protections worked.\n");
    } else {
        printk("malloc() protections failed.\n");
        panic(MEMORY_ISSUE);
        free(unsafe_buf);
    }
    printk("Creating buffer with a safe amount of memory...");
    safe_buf = malloc(1);
   if (safe_buf == NULL) {
        printk("malloc() failed\n");
        panic(MEMORY_ISSUE);
    } else {
        printk("malloc() worked.\n");
		free(safe_buf);
    }
}
