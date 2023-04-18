#include <kio.h>
#include <vga.h>
#include <memory.h>

static uint8_t *pixel_vga_buffer;

// counter to store new lines
// fore & back color values
static uint8_t pixel_color = GREEN;
#define PIXEL_VGA (0xA0000)
#define PIXEL_VGA_LENGTH (200)
#define PIXEL_VGA_WIDTH (320)
#define PIXEL_VGA_PIXEL_AMOUNT (320 * 200)
#define PIXEL_VGA_END (PIXEL_VGA + PIXEL_VGA_PIXEL_AMOUNT)
static void place_pixel(int32_t x, int32_t y, uint8_t vga_color) {
    /* This only works on VGA buffers that use a
     * linear-style memory layout. */
    uint8_t *location = (uint8_t *)PIXEL_VGA + PIXEL_VGA_WIDTH * y + x;
    *location = vga_color;
}
void test_vga_color(void) {
        place_pixel(0, 0, RED);
        place_pixel(0, 1, RED);
        place_pixel(0, 2, RED);
        place_pixel(0, 3, RED);
        place_pixel(0, 4, RED);
}

// clear video buffer array
static void clear_pixel_vga_buffer(uint8_t vga_color) {
    for (int x = 0; x < PIXEL_VGA_WIDTH; x++) {
        for (int y = 0; y < PIXEL_VGA_LENGTH; y++) {
            place_pixel(x, y, vga_color);
        }
    }
}
void init_pixel_vga(uint8_t vga_color) {

    pixel_vga_buffer = (uint8_t *)PIXEL_VGA;
    /* VGA register stuff for
     * 640x480. Not used, but
     * kept here. */
    /*outb(0x3ce, 0x8);
    outb(0x3cf, 0xff);
    outb(0x3ce, 5);
    outb(0x3cf, 2);*/
    clear_pixel_vga_buffer(vga_color);
}




void reset_pixel_memory(void) {
    pixel_vga_buffer = (uint8_t *)PIXEL_VGA;
    clear_pixel_vga_buffer(pixel_color);
}
