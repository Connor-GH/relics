#ifndef MEMORY_H
#define MEMORY_H
#include <typedefs.h>
#include <orbit.h>

/* This file is a working project;
 * nothing in memory.c is complete. */

uint32_t
init_all_memory(void);
size_t
all_mem_available_bytes(void);
void
mem_into_pages(void);

void *
malloc(size_t size);
void
free(void *__owned ptr);

#endif /* MEMORY_H */
