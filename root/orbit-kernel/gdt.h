#ifndef GDT_H
#define GDT_H
#include <typedefs.h>
struct gdt_ptr {
	uint16_t limit;
	uintptr_t base;
} __attribute__((packed));
void
gdt_init(void);
#endif /* GDT_H */
