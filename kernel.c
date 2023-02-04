#include <stddef.h>
#include "util/include/kio.h"
#include "util/include/string.h"
#include "util/include/keyboard.h"
#include "util/include/memory.h"
#include "32bit/include/test_memory.h"
#include "util/include/math.h"

/*
 * sleep() uses NOPs here. It is
 * entirely processor-dependant.
 */

#define TRANSITION(x) \
\
for (int i = x; i > 0; i--) { \
    print_string("Going to input test screen in "); \
    print_int(i); \
    print_string("...\n"); \
    sleep(0x17FFFFFF); \
} do{}while(0)

int init_kernel(void);
int init_kernel(void) {
    const char relics[52] =
        "################\n"
        "# Relics 0.0.1 #\n"
        "################\n";
    init_vga(WHITE, BLUE);

/*
 * If you ever run into
 * weird space-dependant
 * issues when printing too much
 * or using too many functions,
 * try reading more from the disk.
 * this is in boot.s, and it's the
 * value moved into dh.
 */
    /*
     * this is too related to a vga driver
     * and will be moved out of kernel
     */
    //test_memory(); /* TODO: investigate */

    print_string("2 to the power of 3 is ");
    print_int(ppowi(2, 3));
    print_string("\n");
    print_string("15 to the power of 2 is ");
    print_int(ppowi(15, 2));
    print_string("\n");
    print_string("16 to the power of 4 is ");
    print_int(ppowi(16, 4));
    print_string("\n");


    TRANSITION(2);

    init_vga(WHITE, BRIGHT_MAGENTA);
    reset_video_memory();


    print_string(relics);
    print_string("Size of relics var: ");
    print_int(strlen(relics));
    print_string("\n");
    print_string("Root-level Extended Linux-like Integrated Computing System\n");
    print_string("Relics can also be called \"Relix\".\n");
    print_string("\n");
    print_string("UNIX Once stood for \"Uniplexed Information [and] Computing System\",\n");
    print_string("and at some point the 'CS' got changed to an 'X'.\n");
    print_string("\n");
    print_string("Welcome to 32-bit protected mode!\n");
    print_string("All of this was printed from the kernel!\n");


    TRANSITION(2);
    init_vga(BRIGHT_GREEN, BLACK);
    reset_video_memory();
    print_string("Test your input here!\n");
    print_string("> ");

    for (int i = 0; i < 99; i++) { test_input(); }

    return 0;

}

