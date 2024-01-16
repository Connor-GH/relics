#ifndef KIO_H
#define KIO_H
#include <typedefs.h>
#include <port_functions.h>
#ifdef KERNEL_LOG
#include <irq.h>
#endif

#define VIDEO_MEMORY 0xB8000
#define BUFSIZE 2000

void
init_vga(uint8_t fore_color, uint8_t back_color);

enum vga_color {
	BLACK,
	BLUE,
	GREEN,
	CYAN,
	RED,
	MAGENTA,
	BROWN,
	GREY,
	DARK_GREY,
	BRIGHT_BLUE,
	BRIGHT_GREEN,
	BRIGHT_CYAN,
	BRIGHT_RED,
	BRIGHT_MAGENTA,
	YELLOW,
	WHITE,
};

void
enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void
disable_cursor(void);

void
reset_video_memory(void);
void
init_pixel_mode(void);

void
printk(const char *restrict format, ...);
void
log_printk(const char *restrict format, ...);
#endif
