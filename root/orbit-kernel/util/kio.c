#include <kio.h>
#include <memory.h>
#include <stdint.h>
#ifdef KERNEL_LOG
#include <irq.h>
#include <cpu.h>
#endif
#include <ps2_keyboard.h>
#include <stdbool.h>
#include <orbit.h>

static uint16_t *vga_buffer;

static uint16_t vga_index = 0;
// counter to store new lines
static uint16_t next_line_index = 1;
// fore & back color values
static uint8_t g_fore_color = WHITE, g_back_color = BLACK;
enum { VGA_LENGTH = 25, VGA_WIDTH = 80, KIO_TABSIZE = 4 };

/* https://wiki.osdev.org/Text_Mode_Cursor */
static void
update_cursor(uint16_t x, uint16_t y)
{
	/* y is next_line_index */
	/* x is vga_index % 80 */
	uint16_t pos = y * VGA_WIDTH + x;

	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t)(pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

/* https://wiki.osdev.org/Text_Mode_Cursor */
void
enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
	outb(0x3D4, 0x0A);
	outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);

	outb(0x3D4, 0x0B);
	outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

void
disable_cursor(void)
{
	outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);
}

static uint16_t
vga_entry(char ch, uint8_t fore_color, uint8_t back_color)
{
	uint16_t ax = 0;
	uint8_t ah = 0, al = 0;

	ah = back_color;
	ah <<= 4;
	ah |= fore_color;
	ax = ah;
	ax <<= 8;
	al = (uint8_t)ch;
	ax |= al;

	return ax;
}

// clear video buffer array
static void
clear_vga_buffer(uint16_t **__owned buffer, uint8_t fore_color, uint8_t back_color)
{
	for (int y = 0; y < VGA_LENGTH; y++) {
		for (int x = 0; x < VGA_WIDTH; x++) {
			(*buffer)[y * VGA_WIDTH + x] = vga_entry(0, fore_color, back_color);
		}
	}
}
void
reset_video_memory(void)
{
	vga_buffer = (uint16_t *)0xB8000;
	clear_vga_buffer(&vga_buffer, g_fore_color, g_back_color);
	vga_index = 0;
	next_line_index = 1;
}
// "scroll" video buffer array
static void
scroll_vga_buffer(uint16_t **__owned buffer, uint8_t fore_color, uint8_t back_color)
{
	for (int y = 1; y < VGA_LENGTH; y++) {
		for (int x = 0; x < VGA_WIDTH; x++) {
			(*buffer)[(y - 1) * VGA_WIDTH + x] = (*buffer)[y * VGA_WIDTH + x];
		}
	}
	for (int x = 0; x < VGA_WIDTH; x++) {
		(*buffer)[(VGA_LENGTH - 1) * VGA_WIDTH + x] =
			vga_entry(0, fore_color, back_color);
	}
	vga_index = (VGA_LENGTH - 1) * VGA_WIDTH;
}

//initialize vga buffer
void
init_vga(uint8_t fore_color, uint8_t back_color)
{
	g_fore_color = fore_color;
	g_back_color = back_color;
	vga_buffer =
		(uint16_t *)VIDEO_MEMORY; //point vga_buffer pointer to VIDEO_MEMORY
	clear_vga_buffer(&vga_buffer, fore_color, back_color); //clear buffer
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
	}
	update_cursor(vga_index % VGA_WIDTH, next_line_index - 1);
}

//assign ascii character to video buffer
static void
print_char(const char ch)
{
	if (ch == 0)
		return;
	if (ch == '\b') {
		vga_index--;
		vga_buffer[vga_index] = vga_entry(' ', g_fore_color, g_back_color);
		update_cursor(vga_index % VGA_WIDTH, next_line_index - 1);
		return;
	}
	if (ch == '\n') {
		print_new_line();
	} else if (ch == '\t') {
		vga_index += KIO_TABSIZE;
		update_cursor(vga_index % VGA_WIDTH, next_line_index - 1);
	} else {
		vga_buffer[vga_index] = vga_entry(ch, g_fore_color, g_back_color);
		vga_index++;
		if (vga_index % VGA_WIDTH == 0)
			next_line_index++;
		update_cursor(vga_index % VGA_WIDTH, next_line_index - 1);
	}
}
static void
putchark(int c)
{
	print_char((char)c);
}

static uint32_t
digit_count(int64_t num)
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
// safety: no 64-bit number will overflow a digit count of 32-bits.
static uint32_t
digit_count_unsigned(uint64_t num)
{
	uint32_t count = 0;
	if (num < 10)
		return 1;
	while (num > 0) {
		count++;
		num /= 10;
	}
	return count;
}

static void
lltoa(long long num, char *number)
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
ulltoa(unsigned long long num, char *number)
{
	unsigned long long dgcount = digit_count_unsigned(num);
	unsigned long long index = dgcount - 1;
	if (num == 0 && dgcount == 1) {
		number[0] = '0';
		number[1] = '\0';
	} else {
		while (num != 0) {
	    char x;
			x = num % 10;
			number[index] = x + '0';
			index--;
			num /= 10;
		}
		number[dgcount] = '\0';
	}
}

// safety(1): zero_pad is guaranteed at callsite to be non-null
// safety(2): num does not need a null byte since it is not
// printed as a string.
static void
decimal_to_base(unsigned long long n, uint32_t base, bool *zero_pad)
{
	const char digits[17] = "0123456789abcdef";
	char num[64];
	int i = 0;
  // tried to divide by zero
  if (base == 0)
    return;
	if (n == 0 && *zero_pad == false) {
		print_char('0');
		return;
	}

	while (n > 0) {
		num[i] = digits[n % (unsigned long long)base];
		n /= (unsigned long long)base;
		i++;
	}
	// zero extension
	if (*zero_pad == true) {
		if (base == 16) {
			while (i < 16) {
				num[i] = '0';
				i++;
			}
		} else if (base == 2) {
			while (i < 64) {
				num[i] = '0';
				i++;
			}
		} else if (base == 10) {
			while (i < 20) { // estimate; 10 does not evenly divide into base 2
				num[i] = '0';
				i++;
			}
		}
	}
	for (int j = i - 1; j >= 0; j--) {
		print_char(num[j]);
	}
	*zero_pad = false;
}

// print string by calling print_char
// safety: str pointer will never be overrun, however
// unsupported bytes could be printed (mostly harmless)
static void
print_string(const char *__owned str)
{
	while (*str) {
		print_char(*str);
		str++;
	}
}

// Print int by converting it into string
// and then printing the string.
static void
print_long_long(long long num)
{
	char str_num[64];
	lltoa(num, str_num);
	print_string(str_num);
}
static void
print_long(long num)
{
	print_long_long((long long)num);
}
static void
print_int(int num)
{
	print_long((long)num);
}
static void
print_unsigned_long_long(unsigned long long num, bool *__inout zero_pad)
{
	decimal_to_base((unsigned long long)num, 10, zero_pad);
}
static void
print_unsigned_long(unsigned long num, bool *__inout zero_pad)
{
	print_unsigned_long_long((unsigned long long)num, zero_pad);
}
static void
print_unsigned_int(unsigned int num, bool *__inout zero_pad)
{
	print_unsigned_long((unsigned long)num, zero_pad);
}

// TODO make more general and extend it to the printk family
// safety(1): every number is null-terminated no matter what branch is taken
// safety(2): `number' is a 64 byte buffer that will never be overrun from
// digit_count_unsigned (which returns a max of ~20
static void
zero_extend_3(uint64_t num, char *__owned number)
{
	unsigned long long dgcount = digit_count_unsigned(num);
	unsigned long long index = dgcount - 1;

	if (num < 10) { // 6 -> 006
		index += 2;
		dgcount += 2;
		number[0] = '0';
		number[1] = '0';
	} else if (num < 100) { // 26 -> 026
		index += 1;
		dgcount += 1;
		number[0] = '0';
	}

	if (num == 0 && dgcount == 1) {
		number[0] = '0';
		number[1] = '0';
		number[2] = '0';
		number[3] = '\0';
	} else {
		while (num != 0) {
	    char x;
			x = num % 10;
			number[index] = x + '0';
			index--;
			num /= 10;
		}
		number[dgcount] = '\0';
	}
}

// buf's size is guaranteed to be 64 bytes at the callsite
// safety: no buffer overflow when these numbers are concatenated
const char *
milliseconds_as_seconds(uint64_t num, char *buf, size_t buf_size)
{
	size_t idx = digit_count_unsigned(num / 1000);

	ulltoa(num / 1000, buf); // whole number of seconds
	buf[idx] = '.'; // dot
	zero_extend_3(num % 1000, buf + idx + 1); // milliseconds
	return buf;
}
#ifdef __SSE_enabled
static void
print_double(double num)
{
	/* Print the num before the decimal point. */
	print_unsigned_long((uint64_t)num);
	print_char('.');
	/* Get only the numbers after the decimal point and print those. */
	/* The amount of digits after the decimal point will be adjustable
     * in the future, when printk is complete. */
	if (((num - (uint64_t)num) * 1000000) == 0)
		print_string("00000");
	else
		print_unsigned_long((num - (uint64_t)num) * 1000000);
}
#endif

#define va_list __builtin_va_list
#define va_arg __builtin_va_arg
/* what works:
 * %c, %i, %d, %%, %u, %s, %lu, %ld, %lld, %llu, %b, %o, %x.
 *
 *  zero padding, but only for unsigned integers at the moment
 *
 * */
static void
actual_print(const char *format, va_list *argp)
{
	bool zero_pad = false;
	while (*format) {
		if (*format == '%') {
format:
			format++;
			switch (*format) {
			case '%':
				print_char('%');
				break;
			case '0': {
				zero_pad = true;
				goto format;
			}
			case 'i':
			case 'd': {
				int d = va_arg(*argp, int);
				print_int(d);
				break;
			}
			case 'l': {
				format++;
				if (!*format)
					break;
				switch (*format) {
				case 'l': {
					format++;
					if (!*format)
						break;
					if (*format == 'u') {
						unsigned long long llu =
							va_arg(*argp, unsigned long long);
						print_unsigned_long_long(llu, &zero_pad);
					} else if (*format == 'd') {
						long long lld = va_arg(*argp, long long);
						print_long_long(lld);
					} else {
						format--;
					}
					break;
				}
				case 'u': {
					unsigned long lu = va_arg(*argp, long unsigned);
					print_unsigned_long(lu, &zero_pad);
					break;
				}
				case 'd': {
					long ld = va_arg(*argp, long);
					print_long(ld);
					break;
				}
				case 'b': {
					unsigned long lb = va_arg(*argp, unsigned long);
					print_char('0');
					print_char('b');
					decimal_to_base(lb, 2, &zero_pad);
					break;
				}
				case 'x': {
					unsigned long x = va_arg(*argp, unsigned long);
					print_char('0');
					print_char('x');
					decimal_to_base(x, 16, &zero_pad);
					break;
				}
				default: {
					format--;
					break;
				}
				}
				break;
			}
			case 'u': {
				unsigned int u = va_arg(*argp, unsigned int);
				print_unsigned_int(u, &zero_pad);
				break;
			}
			case 'b': {
				unsigned int b = va_arg(*argp, unsigned int);
				print_char('0');
				print_char('b');
				decimal_to_base(b, 2, &zero_pad);
				break;
			}
			case 'X': {
				unsigned int x = va_arg(*argp, unsigned int);
				print_char('0');
				print_char('X');
				decimal_to_base(x, 16, &zero_pad);
				break;
			}
			case 'x': {
				unsigned int x = va_arg(*argp, unsigned int);
				print_char('0');
				print_char('x');
				decimal_to_base(x, 16, &zero_pad);
				break;
			}
			case 'o': {
				unsigned int o = va_arg(*argp, unsigned int);
				print_char('0');
				print_char('o');
				decimal_to_base(o, 8, &zero_pad);
				break;
			}
      case 'p': {
        void *ptr = va_arg(*argp, void *);
        print_char('0');
        print_char('x');
        decimal_to_base((uintptr_t)ptr, 16, &zero_pad);
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
#ifdef __SSE_enabled
			case 'f': {
				/* must be double due to
                           * va_arg's promotion model */
				double f = va_arg(*argp, double);
				print_double(f);
				break;
			}
#endif

			default: {
        printk("Format unimplemented: %c\n", *format);
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
printk(const char *__borrowed restrict format, ...)
{
	va_list listp;
	va_start(listp, format);
	actual_print(format, &listp);
	va_end(listp);
}

void
log_printk(const char *__borrowed restrict format, ...)
{
#ifdef KERNEL_LOG
  char buf[64];
	va_list listp;
  va_start(listp, format);
	printk("[%s] ", PIT_IRQ_timer_get_current_time_since_boot(buf));
	actual_print(format, &listp);
	va_end(listp);
#endif
}

int
getchark(void)
{
	/* keep waiting to get a char until
	 * one comes in, then grab it */
	char c = get_most_recent_char();
	while (c == '\0') {
		c = get_most_recent_char();
	}
	return c;
}
