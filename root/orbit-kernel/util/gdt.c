#include <asm/wrappers.h>
#include <kio.h>
#include <gdt.h>
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

struct GDT_struct {
uint64_t limit1: 16,
		 base1: 24,
		 access: 8,
		 limit2: 4,
		 flags: 4,
		 base2: 8;
} __attribute__((packed));

static struct gdt_ptr gdtr;


static struct GDT_struct _gdt_descriptors[GDT_MAX_ENTRIES];

static void fill_gdt_descriptor_item(size_t vector, uint32_t base,
		uint32_t limit, uint8_t access, uint8_t flags) {
	struct GDT_struct *descriptor = &_gdt_descriptors[vector];
	descriptor->base1 = base & 0x00FFFFFF; // only keep bottom 24 bits
	descriptor->base2 = (uint8_t)((base & 0xFF000000) >> 24);
	descriptor->limit1 = (uint16_t)(limit & 0x0000FFFF);
	descriptor->limit2 = (limit & 0x000F0000) >> 16;
	descriptor->access = access;
	descriptor->flags = flags & 0x0F;
}

void gdt_init(void) {
	// null
	fill_gdt_descriptor_item(0, 0, 0, 0, 0);
	// kernel_code_segm
	fill_gdt_descriptor_item(1, 0, 0xFFFFF, PRESENT_BIT | DPL(0, 0) | DESCRIPTOR_TYPE_CODE_OR_DATA_SEGMENT |
			EXECUTABLE_BIT_CODE | DIRECTION_CONFORMING_BIT(0) | READ_WRITE_BIT(1), CODE_SEGMENT);
	// kernel_data_segm
	fill_gdt_descriptor_item(2, 0, 0xFFFFF, PRESENT_BIT | DPL(0, 0) | DESCRIPTOR_TYPE_CODE_OR_DATA_SEGMENT |
			EXECUTABLE_BIT_DATA | DIRECTION_CONFORMING_BIT(0) | READ_WRITE_BIT(1), DATA_SEGMENT);
	// user_code_segm
	fill_gdt_descriptor_item(3, 0, 0xFFFFF, PRESENT_BIT | DPL(1, 1) | DESCRIPTOR_TYPE_CODE_OR_DATA_SEGMENT |
			EXECUTABLE_BIT_CODE | DIRECTION_CONFORMING_BIT(0) | READ_WRITE_BIT(1), CODE_SEGMENT);
	// user_data_segm
	fill_gdt_descriptor_item(4, 0, 0xFFFFF, PRESENT_BIT | DPL(1, 1) | DESCRIPTOR_TYPE_CODE_OR_DATA_SEGMENT |
			EXECUTABLE_BIT_DATA | DIRECTION_CONFORMING_BIT(0) | READ_WRITE_BIT(1), DATA_SEGMENT);
	//fill_gdt_descriptor_item(gdt->tss_segm, 0, 0, 0);

	gdtr.size = sizeof(struct GDT_struct) * GDT_MAX_ENTRIES -1;
	gdtr.limit = (uintptr_t)&_gdt_descriptors[0];

	gdt_reload(&gdtr);
	log_printk("GDT initialized from C\n");
}
