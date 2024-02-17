#ifndef E820_H
#define E820_H
#include <orbit.h>
#include <inttypes.h>

enum e820_type {
	E820_TYPE_USABLE = 1,
	E820_TYPE_RESERVED = 2,
	E820_TYPE_ACPI = 3,
	E820_TYPE_NVS = 4,
	E820_TYPE_UNUSABLE = 5,
	E820_TYPE_PMEM = 7
};

#define E820_MAX_ENTRIES                      \
	100 // arbitrary, but we want to be below \
		// KERNEL_LOCATION
struct e820_map_64 {
	uint64_t baseaddr;
	uint64_t length;
	uint32_t type;
} ATTR(packed);
void
get_mem_map(void);

#endif /* E820_H */
