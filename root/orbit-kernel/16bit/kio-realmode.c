#include <kio-realmode.h>
#include <memory.h>

// TODO file unused
static uint16_t *vga_buffer;

static uint32_t vga_index = 0;
// counter to store new lines
static uint32_t next_line_index = 1;
// fore & back color values
static uint8_t g_fore_color = WHITE, g_back_color = BLACK;
#define VGA_LENGTH 25
#define VGA_WIDTH 80
#define KIO_TABSIZE 4

#if 0
/* https://wiki.osdev.org/Text_Mode_Cursor */
static void realmode_update_cursor(int x, int y) {
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

#endif

static uint16_t
realmode_vga_entry(unsigned char ch, uint8_t fore_color, uint8_t back_color)
{
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
static void
realmode_clear_vga_buffer(uint16_t **buffer, uint8_t fore_color,
						  uint8_t back_color)
{
	for (int y = 0; y < VGA_LENGTH; y++) {
		for (int x = 0; x < VGA_WIDTH; x++) {
			(*buffer)[y * VGA_WIDTH + x] =
				realmode_vga_entry(0, fore_color, back_color);
		}
	}
}
void
realmode_reset_video_memory(void)
{
	vga_buffer = (uint16_t *)VIDEO_MEMORY;
	realmode_clear_vga_buffer(&vga_buffer, g_fore_color, g_back_color);
	vga_index = 0;
	next_line_index = 1;
}
// "scroll" video buffer array
static void
scroll_vga_buffer(uint16_t **buffer, uint8_t fore_color, uint8_t back_color)
{
	for (int y = 1; y < VGA_LENGTH; y++) {
		for (int x = 0; x < VGA_WIDTH; x++) {
			(*buffer)[(y - 1) * VGA_WIDTH + x] = (*buffer)[y * VGA_WIDTH + x];
		}
	}
	for (int x = 0; x < VGA_WIDTH; x++) {
		(*buffer)[(VGA_LENGTH - 1) * VGA_WIDTH + x] =
			realmode_vga_entry(0, fore_color, back_color);
	}
	vga_index = (VGA_LENGTH - 1) * VGA_WIDTH;
}

//initialize vga buffer
void
realmode_init_vga(uint8_t fore_color, uint8_t back_color)
{
	g_fore_color = fore_color;
	g_back_color = back_color;
	realmode_reset_video_memory();
}

static void
print_new_line(void)
{
	/* Scrolls the screen.
     * The alternative is clearning
     * the buffer and setting next line index to 1.
     */
	if (next_line_index >= VGA_LENGTH) {
		scroll_vga_buffer(&vga_buffer, g_fore_color, g_back_color);
	} else {
		vga_index = VGA_WIDTH * next_line_index;
		next_line_index++;
		//update_cursor(vga_index % VGA_WIDTH, next_line_index-1);
	}
}

//assign ascii character to video buffer
static void
print_char(const unsigned char ch)
{
	if (ch == '\n') {
		print_new_line();
	} else if (ch == '\t') {
		vga_index += KIO_TABSIZE;
		//update_cursor(vga_index % VGA_WIDTH, next_line_index-1);
	} else {
		vga_buffer[vga_index] =
			realmode_vga_entry(ch, g_fore_color, g_back_color);
		vga_index++;
		//update_cursor(vga_index % VGA_WIDTH, next_line_index-1);
	}
}
static int
putchark(int c)
{
	if ((c > 255) || (c < 0))
		return -1;
	else
		print_char((unsigned char)c);
}

static uint32_t
digit_count(int32_t num)
{
	uint32_t count = 0;
	if (num == 0)
		return 1;
	if (num < 0) {
		num *= -1;
		count++;
	}
	while (num > 0) {
		count++;
		num /= 10;
	}
	return count;
}
static uint32_t
digit_count_unsigned(uint32_t num)
{
	uint32_t count = 0;
	if (num == 0)
		return 1;
	while (num > 0) {
		count++;
		num /= 10;
	}
	return count;
}

static void
itoa(int num, char *number)
{
	unsigned int dgcount = digit_count(num);
	unsigned int index = dgcount - 1;
	char x;
	if (num == 0 && dgcount == 1) {
		number[0] = '0';
		number[1] = '\0';
	} else if (num < 0) {
		number[0] = '-';
		num *= -1;
		goto getnum;
	} else {
getnum:
		while (num != 0) {
			x = num % 10;
			number[index] = x + '0';
			index--;
			num = num / 10;
		}
		number[dgcount] = '\0';
	}
}
static void
uitoa(unsigned int num, char *number)
{
	unsigned int dgcount = digit_count_unsigned(num);
	unsigned int index = dgcount - 1;
	char x;
	if (num == 0 && dgcount == 1) {
		number[0] = '0';
		number[1] = '\0';
	} else {
		while (num != 0) {
			x = num % 10;
			number[index] = x + '0';
			index--;
			num /= 10;
		}
		number[dgcount] = '\0';
	}
}

//print string by calling print_char
static void
print_string(const unsigned char *str)
{
	while (*str) {
		print_char(*str);
		str++;
	}
}

// Print int by converting it into string
// and then printing the string.
static void
print_int(int num)
{
	char str_num[digit_count(num) + 1];
	itoa(num, str_num);
	print_string(str_num);
}
static void
print_unsigned_int(unsigned int num)
{
	char str_num[digit_count_unsigned(num) + 1];
	uitoa(num, str_num);
	print_string(str_num);
}

static void
decimal_to_base(unsigned int n, int base)
{
	const char digits[17] = "0123456789abcdef";
	char num[64];
	int i = 0;
	if (n == 0) {
		print_char('0');
		return;
	}
	while (n > 0) {
		num[i] = digits[n % base];
		n /= base;
		i++;
	}
	for (int j = i - 1; j >= 0; j--)
		print_char(num[j]);
}

#define va_list __builtin_va_list
#define va_arg __builtin_va_arg
/* what works:
 * %c, %i, %d, %%, %u, %s, %b, %o, %x.
 *
 * No zero extension, no "alternative form", etc.
 *
 * */
static void
_print(const char *format, va_list *argp)
{
	while (*format) {
		if (*format == '%') {
			format++;
			switch (*format) {
			case '%':
				print_char('%');
				break;
			case 'i':
			case 'd': {
				int d = va_arg(*argp, int);
				print_int(d);
				break;
			}
			case 'u': {
				unsigned int u = va_arg(*argp, unsigned int);
				print_unsigned_int(u);
				break;
			}
			case 'b': {
				unsigned int b = va_arg(*argp, unsigned int);
				if (b != 0) {
					print_char('0');
					print_char('b');
				}
				decimal_to_base(b, 2);
				break;
			}
			case 'X': {
				unsigned int x = va_arg(*argp, unsigned int);
				if (x != 0) {
					print_char('0');
					print_char('X');
				}
				decimal_to_base(x, 16);
				break;
			}
			case 'x': {
				unsigned int x = va_arg(*argp, unsigned int);
				if (x != 0) {
					print_char('0');
					print_char('x');
				}
				decimal_to_base(x, 16);
				break;
			}
			case 'o': {
				unsigned int o = va_arg(*argp, unsigned int);
				if (o != 0) {
					print_char('0');
					print_char('o');
				}
				decimal_to_base(o, 8);
				break;
			}
			case 'c': {
				int c = va_arg(*argp, int);
				putchark(c);
				break;
			}
			case 's': {
				char *s = va_arg(*argp, char *);
				print_string(s);
				break;
			}
			default: {
				format--;
				break;
			}
			}
		} else {
			print_char(*format);
		}
		format++;
	}
}
#define va_start __builtin_va_start
#define va_end __builtin_va_end
void
realmode_printk(const char *restrict format, ...)
{
	va_list listp;
	va_start(listp, format);
	_print(format, &listp);
	va_end(listp);
}
