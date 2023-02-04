#include "include/kio.h"
#include "include/memory.h"

static unsigned short* vga_buffer;

static uint32_t vga_index = 0;
// counter to store new lines
static uint32_t next_line_index = 1;
// fore & back color values
uint8_t g_fore_color = WHITE, g_back_color = BLACK;

void reset_video_memory(void) {
    vga_buffer = (uint16_t *)0xB8000;
    clear_vga_buffer(&vga_buffer, g_fore_color, g_back_color);
    vga_index = 0;
    next_line_index = 1;
}

uint16_t vga_entry(unsigned char ch, uint8_t fore_color, uint8_t back_color) {

    uint16_t ax = 0;
    uint8_t ah = 0, al = 0;

    ah = back_color;
    ah <<= 4;
    ah |= fore_color;
    ax = ah;
    ax <<= 8;
    al = ch;
    ax |= al;

    return ax;
}

// clear video buffer array
void clear_vga_buffer(uint16_t **buffer, uint8_t fore_color, uint8_t back_color) {
    for(int y = 0; y < 25; y++) {
        for(int x = 0; x < 80; x++) {
            (*buffer)[y * 80 + x] = vga_entry(0, fore_color, back_color);
        }
    }
}

//initialize vga buffer
void init_vga(uint8_t fore_color, uint8_t back_color) {
    g_fore_color = fore_color;
    g_back_color = back_color;
    vga_buffer = (uint16_t*)VIDEO_MEMORY;  //point vga_buffer pointer to VIDEO_MEMORY
    clear_vga_buffer(&vga_buffer, fore_color, back_color);  //clear buffer
}



void print_new_line(void) {

    if(next_line_index >= 55) {

        next_line_index = 1;
        clear_vga_buffer(&vga_buffer, g_fore_color, g_back_color);
    }

    vga_index = 80*next_line_index;
    next_line_index++;
}

//assign ascii character to video buffer
void print_char(const char ch) {

    vga_buffer[vga_index] = vga_entry(ch, g_fore_color, g_back_color);
    vga_index++;
}

uint32_t digit_count(int num) {

    uint32_t count = 0;
    if(num == 0) return 1;

    while(num > 0) {

        count++;
        num = num/10;
    }
    return count;
}

void itoa(int num, char *number) {

    unsigned int dgcount = digit_count(num);
    unsigned int index = dgcount - 1;
    char x;
    if(num == 0 && dgcount == 1) {

        number[0] = '0';
        number[1] = '\0';
    }
    else {

    while (num != 0) {

        x = num % 10;
        number[index] = x + '0';
        index--;
        num = num / 10;
        }
    number[dgcount] = '\0';
    }
}

//print string by calling print_char
void print_string(const char *str) {

    uint32_t index = 0;
    while(str[index]) {

        if (str[index] == '\n') print_new_line();

    else print_char(str[index]);

    index++;
  }
}

//print int by converting it into string
//& then printing string
void print_int(int num) {

    char str_num[digit_count(num)+1];
    itoa(num, str_num);
    print_string(str_num);
}

 void decimal_to_binary(int n) {
    // array to store binary number
    int binary_num[32];
    int i = 0;
    while (n > 0) {
        binary_num[i] = n % 2;
        n = n / 2;
        i++;
    }
    // printing binary array in reverse order
    for (int j = i - 1; j >= 0; j--)
        print_int(binary_num[j]);
}
