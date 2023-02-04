#ifndef KIO_H
#define KIO_H
#include "typedefs.h"

#define VIDEO_MEMORY 0xB8000
//#define BUFSIZE 2200
#define BUFSIZE 2000

extern uint8_t g_fore_color, g_back_color;

uint16_t vga_entry(unsigned char ch, uint8_t fore_color, uint8_t back_color);
void clear_vga_buffer(uint16_t **buffer, uint8_t fore_color, uint8_t back_color);
void init_vga(uint8_t fore_color, uint8_t back_color);


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

void print_new_line(void);
void print_char(const char ch);
void reset_video_memory(void);

uint32_t digit_count(int num);
void itoa(int num, char *number);

void print_string(const char *str);
void print_int(int num);
void decimal_to_binary(int n);
#endif

