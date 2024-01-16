#ifndef MEMORY_H
#define MEMORY_H
#include <typedefs.h>

/* This file is a working project;
 * nothing in memory.c is complete. */

uint32_t
init_all_memory(void);
int
all_mem_available_bytes(void);
void
mem_into_pages(void);

void *
malloc(size_t size);
void
free(void *ptr);

#endif /* MEMORY_H */
