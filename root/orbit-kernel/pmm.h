#ifndef PMM_H
#define PMM_H
#include <e820.h>

void pmem_map(struct e820_map_64 *e820_map, uint32_t e820_count);
#endif /* PMM_H  */
