#include <stdarg.h>
#include <println_c.h>

//extern int
//println_d(const char *fmt, va_list *argp);

int
println(const char *__borrowed fmt, ...)
{
	int written = 0;
	va_list list;
	va_start(list, fmt);
	//written = println_d(fmt, &list);
	va_end(list);
	return written;
}
