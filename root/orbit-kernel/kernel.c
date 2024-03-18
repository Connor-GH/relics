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


ATTR(noreturn) int init_kernel(void);
int
println_d(const char __owned *fmt, ...);

ATTR(noreturn)
int
init_kernel(void)
{
	ASM(".code64\t\n");
	printk("--[Relics version %s]--\n", VERSION);
	printk("Raw Exokernel-LibraryOS Integrated Computing System\n");
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
	gdt_init();
	idt_init();

#if 0
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
#endif
	enable_interrupts();
	reprogram_timer(1000); // tick every ms
	printk("Sleeping for one second!\n");
	millisleep(1000);
  get_mem_map();
  log_printk("Halting!\n");
  ASM("cli; hlt");
  // println_d("Fantastic!");
	relics_shell("> ");
	for (;;) {
		__asm__ __volatile__("hlt");
	}
}
