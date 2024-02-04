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
#define CODE_SEGMENT 0xA
#define DATA_SEGMENT 0xC
#define GDT_MAX_ENTRIES 8192
extern void gdt_reload(struct gdt_ptr *);

struct bits {
	uint8_t limit_high : 4,
			flags : 4;
} __attribute__((packed));
struct GDT_struct {
#if 0
	uint64_t limit1: 16,
		 base1: 24,
		 access: 8,
		 limit2: 4,
		 flags: 4,
		 base2: 8;
#endif
#if 1
	uint64_t base2: 8,
		 flags: 4,
		 limit2: 4,
		 access: 8,
		 base1: 24,
		 limit1: 16;
#endif
#if 0
	uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t base_high;
	uint8_t granularity;
#endif
} __attribute__((packed));

static struct gdt_ptr gdtr;


__attribute__((aligned(16)))
static struct GDT_struct _gdt_descriptors[GDT_MAX_ENTRIES];

//static void fill_gdt_descriptor_item(size_t vector, uint32_t base,
//		uint32_t limit, uint8_t access, uint8_t flags) {
static void fill_gdt_descriptor_item(size_t vector, uint32_t base,
		uint32_t limit, uint8_t access, uint8_t flags) {
	struct GDT_struct *descriptor = &_gdt_descriptors[vector];
#if 1
	descriptor->base1 = base & 0x00FFFFFF; // only keep bottom 24 bits
	descriptor->base2 = (uint8_t)((base & 0xFF000000) >> 24);
	descriptor->limit1 = (uint16_t)(limit & 0x0000FFFF);
	descriptor->limit2 = (limit & 0x000F0000) >> 16;
	descriptor->access = access;
	descriptor->flags = flags & 0x0F;
#endif
#if 0
	descriptor->limit_low = (uint16_t)(limit & 0xFFFF);
	descriptor->base_low = (uint16_t)(base & 0xFFFF);
    descriptor->base_middle = (uint8_t)((base >> 16) & 0xFF);
    descriptor->access = access;
   // descriptor->bits.limit_high = (limit >> 16) & 0xF;
    //descriptor->bits.flags = (limit >> 16) & 0xF0;
	descriptor->base_high = (base >> 24) & 0xFF;
	descriptor->granularity = (limit >> 16) & 0x0F;
	descriptor->granularity |= flags & 0xF0;
#endif


}

static void
sanity_check_all_structures(void)
{
	printk("gdtr: Expected size: 80; Real size: %lu\n", sizeof(gdtr) * 8);
	printk("gdtr: .base = %x, .limit = %lu\n", gdtr.base, gdtr.limit);

	printk("GDT_struct: Expected size: 64; Real size: %lu\n",
		   sizeof(_gdt_descriptors[0]) * 8);
}
extern uintptr_t ret_gdt_cs(void);
extern uintptr_t ret_gdt_ds(void);


void gdt_init(void) {
	memset(_gdt_descriptors, 0, sizeof(struct GDT_struct) * GDT_MAX_ENTRIES - 1);
	// null
	fill_gdt_descriptor_item(0, 0, 0, 0, 0);
	// access: 10011010 -> 0x9A
	fill_gdt_descriptor_item(1, 0, 0xFFFFF, PRESENT_BIT | DPL(0, 0) | DESCRIPTOR_TYPE_CODE_OR_DATA_SEGMENT |
			EXECUTABLE_BIT_CODE | DIRECTION_CONFORMING_BIT(0) | READ_WRITE_BIT(1) | ACCESS_BIT(0), CODE_SEGMENT);
	// kernel_data_segm
	// access: 10010010 -> 0x92
	fill_gdt_descriptor_item(2, 0, 0xFFFFF, PRESENT_BIT | DPL(0, 0) | DESCRIPTOR_TYPE_CODE_OR_DATA_SEGMENT |
			EXECUTABLE_BIT_DATA | DIRECTION_CONFORMING_BIT(0) | READ_WRITE_BIT(1) | ACCESS_BIT(0), DATA_SEGMENT);
	// user_code_segm
	//fill_gdt_descriptor_item(3, 0, 0xFFFFF, PRESENT_BIT | DPL(1, 1) | DESCRIPTOR_TYPE_CODE_OR_DATA_SEGMENT |
//			EXECUTABLE_BIT_CODE | DIRECTION_CONFORMING_BIT(0) | READ_WRITE_BIT(1) | ACCESS_BIT(0), CODE_SEGMENT);
	// user_data_segm
//	fill_gdt_descriptor_item(4, 0, 0xFFFFF, PRESENT_BIT | DPL(1, 1) | DESCRIPTOR_TYPE_CODE_OR_DATA_SEGMENT |
//			EXECUTABLE_BIT_DATA | DIRECTION_CONFORMING_BIT(0) | READ_WRITE_BIT(1) | ACCESS_BIT(0), DATA_SEGMENT);
	//fill_gdt_descriptor_item(gdt->tss_segm, 0, 0, 0);

	gdtr.limit = (sizeof(struct GDT_struct) * GDT_MAX_ENTRIES) - 1;
	gdtr.base = (uintptr_t)&_gdt_descriptors[0];

	union Bits {
		struct GDT_struct gdt;
		uint64_t bits;
	};
	union Bits cs;
	cs.gdt = _gdt_descriptors[1];
	union Bits ds;
	ds.gdt = _gdt_descriptors[2];
	//gdt_reload(&gdtr);
	log_printk("GDT initialized from C\n");
	log_printk("GDT_CS from C: %lb\n", cs.bits);
	log_printk("GDT_CS from ASM: %lb\nOld: %lb\n", ret_gdt_cs(), 0x00209A0000000000);
	log_printk("GDT_DS from C: %lb\n", ds.bits);
	log_printk("GDT_DS from ASM: %lb\nOld: %lb\n",ret_gdt_ds(), 158329674399744);
	sanity_check_all_structures();
}
