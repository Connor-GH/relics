#include <asm/wrappers.h>
#include <kio.h>
#include <gdt.h>
#include <orbit-kernel/string.h>
#include <typedefs.h>

#define PRESENT_BIT 1 << 7
#define DPL(x, y) (x << 6 | y << 5)
#define DESCRIPTOR_TYPE_CODE_OR_DATA_SEGMENT 1 << 4
#define DESCRIPTOR_TYPE_TSS_SEGMENT 0 << 4
#define EXECUTABLE_BIT_DATA 0 << 3
#define EXECUTABLE_BIT_CODE 1 << 3
#define DIRECTION_CONFORMING_BIT(x)  x << 2
#define READ_WRITE_BIT(x) x << 1
#define ACCESS_BIT(x) x
#define CODE_SEGMENT 0xA // 1010 -> 4KiB, 64 bit
#define DATA_SEGMENT 0xA
#define GDT_MAX_ENTRIES 8192
extern void gdt_reload(struct gdt_ptr *);

static struct gdt_ptr gdtr;


__attribute__((aligned(16)))
static uint64_t _gdt_descriptors[GDT_MAX_ENTRIES];

#define set_bit_value(number, bit, value) (number | (value << bit))

uint64_t set_several_bits(uint64_t num,
		short idx_inclusive, short idx_exclusive, uint64_t pattern) {
	short i = 0;
	for (pattern; idx_inclusive < idx_exclusive; idx_inclusive++, i++) {
		num = set_bit_value(num, idx_inclusive, ((pattern & (1 << i)) >> i));
	}
	return num;
}

static void fill_gdt_descriptor_item(size_t vector, uint32_t base,
		uint32_t limit, uint8_t access, uint8_t flags) {
	_gdt_descriptors[vector] = set_several_bits(_gdt_descriptors[vector], 0, 16, (uint16_t)(limit & 0x0000FFFF));
	_gdt_descriptors[vector] = set_several_bits(_gdt_descriptors[vector], 16, 40, base & 0xFFFFFF);
	_gdt_descriptors[vector] = set_several_bits(_gdt_descriptors[vector], 40, 48, access);
	_gdt_descriptors[vector] = set_several_bits(_gdt_descriptors[vector], 48, 52, (limit >> 16) & 0xF);
	_gdt_descriptors[vector] = set_several_bits(_gdt_descriptors[vector], 52, 56, flags & 0xF);
	_gdt_descriptors[vector] = set_several_bits(_gdt_descriptors[vector], 56, 64, (uint8_t)(base >> 24));
}

static void
sanity_check_all_structures(void)
{
	if ((sizeof(gdtr) * 8) != 80) {
		printk("ERROR: gdtr: not correct size\n");
		ASM("cli; hlt\t\n");
	}
}

extern uintptr_t ret_gdt_cs(void);
extern uintptr_t ret_gdt_ds(void);


void gdt_init(void) {
	memset(_gdt_descriptors, 0, sizeof(uint64_t) * GDT_MAX_ENTRIES - 1);
	fill_gdt_descriptor_item(0, 0, 0, 0, 0);
	fill_gdt_descriptor_item(1, 0, 0xFFFFF, PRESENT_BIT | DPL(0, 0) | DESCRIPTOR_TYPE_CODE_OR_DATA_SEGMENT |
			EXECUTABLE_BIT_CODE | DIRECTION_CONFORMING_BIT(0) | READ_WRITE_BIT(1) | ACCESS_BIT(0), CODE_SEGMENT);
	fill_gdt_descriptor_item(2, 0, 0xFFFFF, PRESENT_BIT | DPL(0, 0) | DESCRIPTOR_TYPE_CODE_OR_DATA_SEGMENT |
			EXECUTABLE_BIT_DATA | DIRECTION_CONFORMING_BIT(0) | READ_WRITE_BIT(1) | ACCESS_BIT(0), DATA_SEGMENT);

	gdtr.limit = (sizeof(uint64_t) * GDT_MAX_ENTRIES) - 1;
	gdtr.base = (uintptr_t)&_gdt_descriptors[0];

	sanity_check_all_structures();
	gdt_reload(&gdtr);
	log_printk("GDT initialized from C\n");
}
