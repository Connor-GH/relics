#include <kio.h>
#include <memory.h>

static uint16_t *vga_buffer;

static uint32_t vga_index = 0;
// counter to store new lines
static uint32_t next_line_index = 1;
// fore & back color values
static uint8_t g_fore_color = WHITE, g_back_color = BLACK;
#define VGA_LENGTH 25
#define VGA_WIDTH 80
#define KIO_TABSIZE 4


/* https://wiki.osdev.org/Text_Mode_Cursor */
static void update_cursor(int x, int y) {
    /* y is next_line_index */
    /* x is vga_index % 80 */
	uint16_t pos = y * VGA_WIDTH + x;

	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

/* https://wiki.osdev.org/Text_Mode_Cursor */
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);

    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

void disable_cursor(void) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}


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
    for(int y = 0; y < VGA_LENGTH; y++) {
        for(int x = 0; x < VGA_WIDTH; x++) {
            (*buffer)[y * VGA_WIDTH + x] = vga_entry(0, fore_color, back_color);
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

    /* TODO scrolling */
    if(next_line_index >= VGA_LENGTH) {

        next_line_index = 1;
        clear_vga_buffer(&vga_buffer, g_fore_color, g_back_color);
    }

    vga_index = VGA_WIDTH*next_line_index;
    next_line_index++;
    update_cursor(vga_index % VGA_WIDTH, next_line_index-1);
}

//assign ascii character to video buffer
void print_char(const char ch) {

    if (ch == '\n') {
        print_new_line();
    } else if (ch == '\t') {
            vga_index += KIO_TABSIZE;
            update_cursor(vga_index % VGA_WIDTH, next_line_index-1);
    } else {
        vga_buffer[vga_index] = vga_entry(ch, g_fore_color, g_back_color);
        vga_index++;
        update_cursor(vga_index % VGA_WIDTH, next_line_index-1);
    }
}

static uint32_t digit_count(int num) {

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
static void uitoa(unsigned int num, char *number) {

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
static void ltoa(long num, char *number) {

    unsigned long dgcount = digit_count(num);
    unsigned long index = dgcount - 1;
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
void ultoa(unsigned long num, char *number) {

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
        print_char(str[index]);
        index++;
    }
}

// Print int by converting it into string
// and then printing the string.
void print_int(int num) {
    char str_num[digit_count(num)+1];
    itoa(num, str_num);
    print_string(str_num);
}
void print_unsigned_long(unsigned long num) {
    char str_num[digit_count(num)+1];
    ultoa(num, str_num);
    print_string(str_num);
}

static void decimal_to_binary(int n) {
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

#define va_list __builtin_va_list
#define va_arg __builtin_va_arg
static void _print(const char *format, va_list *argp) {
    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format++) {
            case '%': print_char('%'); break;
            case 'i':
            case 'd': {
                          int d = va_arg(*argp, int);
                          print_int(d);
                          break;
                      }
            case 'u': {
                          format++;
                          switch (*format) {
                          case 'l': {
                                        format++;
                                        if (*format == 'l') {/* %ull */
                                            uint64_t ull = va_arg(*argp, unsigned long long);
                                            print_unsigned_long(ull); /* TODO print_unsigned_long_long */

                                        } else {
                                            format--;
                                            uint64_t ul = va_arg(*argp, unsigned long);
                                            print_unsigned_long(ul);
                                        }
                                      break;
                                    }
                            /* unsigned int */
                            default:  {
                                      format--;
                                      unsigned int d = va_arg(*argp, unsigned int);
                                      print_int(d); /* TODO print_unsigned_int */
                                      break;
                                      }
                          }
                        break;
                      }
            case 'c': {
                          char c = va_arg(*argp, char);
                          print_char(c);
                          break;
                      }

            default: break;
            }
        } else {
            print_char(*format);
        }
        format++;
    }
}
#define va_start __builtin_va_start
#define va_end __builtin_va_end
void printk(const char *restrict format, ...) {
    va_list listp;
    va_start(listp, format);
    _print(format, &listp);
    va_end(listp);
}
