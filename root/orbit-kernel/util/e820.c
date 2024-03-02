#include <e820.h>
#include <panic.h>
#include <inttypes.h>
#include <kio.h>

extern struct e820_map_64 *e820_map;
struct e820_map_64 *e820_map = (struct e820_map_64 *)0x500;

extern uint32_t e820_count;
uint32_t e820_count = 0;

static const char *
e820_type_human_name(uint32_t type)
{
	switch (type) {
	case E820_TYPE_USABLE:
		return "usable";
	case E820_TYPE_RESERVED:
		return "reserved";
	case E820_TYPE_ACPI:
		return "ACPI data";
	case E820_TYPE_NVS:
		return "NVS";
	case E820_TYPE_UNUSABLE:
		return "unusable";
	case E820_TYPE_PMEM:
		return "pmem";
	default:
		return "unknown type";
	}
}

static void
iterate_over_map(uint32_t count, struct e820_map_64 *map)
{
	for (size_t i = 0; i < count; i++) {
		log_printk("%0lx-%0lx %s\n", map[i].baseaddr,
				   map[i].baseaddr + map[i].length - 1,
				   e820_type_human_name(map[i].entry_type));
	}
}

void
get_mem_map(void)
{
	uint32_t *e820_entry_count = (uint32_t *)0x496;
	if (*e820_entry_count > E820_MAX_ENTRIES)
		panic2(GENERIC_ISSUE, "Too many E820 entries!");

	log_printk("E820 Memory Map:\n");
	log_printk("entry count: %d\n", *e820_entry_count);
	e820_count = *e820_entry_count;

	iterate_over_map(e820_count, e820_map);
	extern void pmem_map_d(void);
	pmem_map_d();
}
