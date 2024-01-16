#include <port_functions.h>
uint8_t
inb(uint16_t port)
{
	uint8_t ret;
	__asm__ __volatile__("inb %%dx, %%al" : "=a"(ret) : "d"(port));

	return ret;
}

void
outb(uint16_t port, uint8_t data)
{
	__asm__ __volatile__("outb %%al, %%dx" : : "d"(port), "a"(data));
}
