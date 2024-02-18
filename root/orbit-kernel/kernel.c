#include <stddef.h>
#include <kio.h>
#include <orbit.h>
#include <libraryOSes/relics-libOS/apps/shell.h>
#include <cpu.h>
#include <vga.h>
#include <idt.h>
#include <pic.h>
#include <pit.h>
#include <gdt.h>
#include <e820.h>

#define TRANSITION(x)                                       \
                                                            \
	for (int i = x; i > 0; i--) {                           \
		printk("Going to input test screen in %d...\n", i); \
		sleep(0x17FFFFFF);                                  \
	}                                                       \
	do {                                                    \
	} while (0)

int ATTR(noreturn) init_kernel(void);
// TODO complete safe_println
extern void
safe_println(const char *fmt, ...);

int
ATTR(noreturn) init_kernel(void)
{
	ASM(".code64\t\n");
	/*
     * If you ever run into
     * weird space-dependent
     * issues when printing too much
     * or using too many functions,
     * try reading more from the disk.
     * this is in bootloader.asm,
     * and it's the value moved into dh.
     */
	init_vga(WHITE, BLACK);
	reset_video_memory();
	//  cpuflags();
	/* init PIC */
	gdt_init();
	idt_init();

#if 0
	test_memory();
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


	TRANSITION(1);

	init_vga(WHITE, BRIGHT_MAGENTA);
	reset_video_memory();

	printk("\n--[Relics version %s]--\n\n", VERSION);
	printk("Raw Exokernel-LibraryOS Integrated Computing System\n");
	printk("Relics can also be called 'Relix'.\n");
	printk("\n");
	printk(
		"UNIX Once stood for \"Uniplexed Information [and] Computing System\",\n");
	printk("and at some point the 'CS' got changed to an 'X'.\n");
	printk("\n");
	printk("Welcome to 64-bit long mode!\n");
	printk("All of this was printed from the kernel!\n");
#endif
	enable_interrupts();
	get_mem_map();
	reprogram_timer(1000); // tick every ms
	printk("Sleeping for one second!\n");
	millisleep(1000);

	safe_println("this");
	relics_shell("> ");
	for (;;) {
		__asm__ __volatile__("hlt");
	}
}
