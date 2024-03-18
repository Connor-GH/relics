#ifndef MALLOC_FREELIST_H
#define MALLOC_FREELIST_H
#include <typedefs.h>
#include <e820.h>

void
kmalloc_init(struct e820_map_64 *usable, size_t count);
#ifdef __malloc_init
void
malloc_addblock(void *addr, size_t size);
#endif
#undef __malloc_init
void *
kmalloc(size_t s);
void
kfree(void *p);
#endif /*MALLOC_FREELIST_H */
