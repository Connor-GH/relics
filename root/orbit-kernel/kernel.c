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
#include <pic.h>

static void
sleep(int x)
{
	for (int i = 0; i < x; i++) {
		__asm__ __volatile__("nop");
	}
}

#define TRANSITION(x)                                       \
                                                            \
	for (int i = x; i > 0; i--) {                           \
		printk("Going to input test screen in %d...\n", i); \
		sleep(0x17FFFFFF);                                  \
	}                                                       \
	do {                                                    \
	} while (0)

static _Bool
are_interrupts_enabled(void)
{
	unsigned long flags;
	__asm__ __volatile__("pushf\t\n"
						 "pop %0\t\n"
						 : "=g"(flags));
	return flags & (1 << 9);
}

int
init_kernel(void);
int
init_kernel(void)
{
	__asm__ __volatile__(".code64\t\n");
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
	idt_init();
	enable_interrupts();
	IRQ_set_mask(0);
	disable_interrupts();
//	__asm__ __volatile__("hlt");
#if 0
	test_memory();
    /* IRQs are now set and remapped.
     * IRQ_0x1 == ISR_0x21 */
    /* masking IRQ 1 will allow
     * keyboard input the "old" way
     * until proper interrupt handling
     * can be added for it. */
    //IRQ_set_mask(1);
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


    //test_fpu(16, 3);
#endif
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

	enable_interrupts();

	TRANSITION(1);
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
		   //"%%f: %f\n",
		   'b', "string", -12, 45, 3147000000, -9000000000000, 18000000000000,
		   23, 17, 256, 4096,
		   7897890); //,
	//355./113);

	for (;;) {
		__asm__ __volatile__("hlt");
	}
	//relics_shell("> ");

	return 0;
}
