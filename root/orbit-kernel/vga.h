#ifndef VGA_H
#define VGA_H
#include <inttypes.h>
void
init_pixel_vga(uint8_t vga_color);
void
reset_pixel_memory(void);
void
test_vga_color(void);
#endif /* VGA_H */
