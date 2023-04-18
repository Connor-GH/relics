#include <stddef.h>
#include <kio.h>
#include <string.h>
#include <keyboard.h>
#include <memory.h>
#include <32bit/include/test_memory.h>
#include <orbit-kernel/math.h>
#include <apploader.h>
#include <libraryOSes/relics-libOS/apps/shell.h>
#include <cpu.h>
#include <vga.h>
#include <idt.h>

/*
 * sleep() uses NOPs here. It is
 * entirely processor-dependant.
 */

#define TRANSITION(x) \
\
for (int i = x; i > 0; i--) { \
    printk("Going to input test screen in %d...\n", i); \
    sleep(0x17FFFF); \
} do{}while(0)


int init_kernel(void);
int init_kernel(void) {

    /*
     * If you ever run into
     * weird space-dependant
     * issues when printing too much
     * or using too many functions,
     * try reading more from the disk.
     * this is in bootloader.asm,
     * and it's the value moved into dh.
     */

    /* FPU works, but the printk format has not been
     * implemented yet. User input has been broken because
     * of IDT enabling. You can get it back by commenting out
     * the "sti" instruction in idt.c. */

    /*
     * if running text mode,
     * only use "vga"-named
     * functions. If using
     * pixel mode, only
     * use "pixel"-named
     * functions. You can read
     * about them in kio.h and
     * vga.h.
     */
    //init_pixel_vga(GREEN);
    //test_vga_color();
    //reset_pixel_memory();
    init_vga(WHITE, BLUE);
    reset_video_memory();


    TRANSITION(2);


    init_vga(WHITE, BRIGHT_MAGENTA);
    reset_video_memory();

    printk("\n--[Relics version %s]--\n\n", VERSION);
    printk("Raw Exokernel-LibraryOS Integrated Computing System\n");
    printk("Relics can also be called 'Relix'.\n");
    printk("\n");
    printk("UNIX Once stood for \"Uniplexed Information [and] Computing System\",\n");
    printk("and at some point the 'CS' got changed to an 'X'.\n");
    printk("\n");
    printk("Welcome to 64-bit long mode!\n");
    printk("All of this was printed from the kernel!\n");

    idt_init();

    TRANSITION(2);
    init_vga(BRIGHT_GREEN, BLACK);
    reset_video_memory();
    printk("Format test:\n"
            "%%c: %c\n"
            "%%s: %s\n"
            "%%d: %d\n"
            "%%i: %i\n"
            "%%u: %u\n"
            "%%lld: %lld\n"
            "%%llu: %llu\n"
            "%%ld: %ld\n"
            "%%lu: %lu\n"
            "%%x: %x\n"
            "%%o: %o\n"
            "%%b: %b\n",
            'b',
            "string",
            -12,
            45,
            3147000000,
            -9000000000000,
            18000000000000,
            23,
            17,
            256,
            4096,
            7897890);



    cpuflags();
    test_fpu(16, 3);

    return 0;

}

