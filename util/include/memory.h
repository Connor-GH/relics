#ifndef MEMORY_H
#define MEMORY_H
#include "typedefs.h"
void init_all_memory(void);
int all_mem_available_bytes(void);
void *malloc(size_t size);
void free(void *ptr);

#endif
