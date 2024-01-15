#include <idt.h>
#include <pic.h>
#include <kio.h>
#include <keyboard.h>
#include <get_ascii_char.h>
#include <port_functions.h>
#define IDT_MAX_DESCRIPTORS 256
#define GDT_CS64_OFFSET ret_gdt_offset()//0x8
#define TSS_IST_EXCEPTION 001
#define ISR(isr_stub, n) extern void isr_stub(void); \
    idt_install_handler(isr_stub, n)

#define IRQ(irq, fn, n) extern void irq(void); \
    extern void __attribute__((no_caller_saved_registers)) fn(regs_t *); \
    irq_handlers[n] = fn; do{}while(0)//\
    //idt_install_handler(irq, n + 0x20)

#define panic_irq(s) printk(s); __asm__ __volatile__("cli; hlt\t\n")
enum {
	GATE_TYPE_INTERRUPT = 0xE,
	GATE_TYPE_TRAP = 0xF,
	GATE_TYPE_CALL = 0xC,
	GATE_TYPE_TASK = 0x5
};
/* https://wiki.osdev.org/Interrupts_Tutorial
 * (based on code licensed under an MIT license) */

extern uintptr_t ret_gdt_offset(void);

typedef struct {
	uint16_t    isr_low;      // The lower 16 bits of the ISR's address
	uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
	struct idt_bits {
		uint16_t ist : 3,
				 zero : 5,
				 type : 5,
				 dpl : 2,
				 p : 1;
	} __attribute__((packed)) bits;
	uint16_t    isr_mid;      // The higher 16 bits of the lower 32 bits of the ISR's address
	uint32_t    isr_high;     // The higher 32 bits of the ISR's address
	uint32_t    reserved;     // Set to zero
} __attribute__((packed)) idt_entry_t; // idt descriptor

__attribute__((aligned(16)))
static idt_entry_t idt[IDT_MAX_DESCRIPTORS] = {0}; /* Create an array of IDT entries;
													  aligned for performance */
__attribute__((aligned(16)))
static void (*irq_handlers[IDT_MAX_DESCRIPTORS])(regs_t *) = {(regs_t *)0};
typedef struct {
    uint16_t        limit; // limit; must be size of IDT descriptors - 1
    uint64_t        base; // base; address of idt descriptors
} __attribute__((packed)) idtr_t;
static void dump_idt_descriptors(void) {
	printk("GDT offset: %x\n", GDT_CS64_OFFSET);
	for (int i = 0; i < 2; i++) {
		printk("isr_low: %x\n"
				"kernel_cs: %x\n"
				"bits.type: %x\n"
				"bits.p: %x\n"
				"isr_mid: %x\n"
				"isr_high: %x\n"
				"reserved: %x\n",
				idt[i].isr_low,
				idt[i].kernel_cs,
				idt[i].bits.type,
				idt[i].bits.p,
				idt[i].isr_mid,
				idt[i].isr_high,
				idt[i].reserved);
	}
}
static void fill_irq_table(void) {
	IRQ(irq0, exception_handler, 0);
	IRQ(irq1, kbd_irq_handler, 1);
	IRQ(irq2, exception_handler, 2);
	IRQ(irq3, exception_handler, 3);
	IRQ(irq4, exception_handler, 4);
	IRQ(irq5, exception_handler, 5);
	IRQ(irq6, exception_handler, 6);
	IRQ(irq7, exception_handler, 7);
	IRQ(irq8, exception_handler, 8);
	IRQ(irq9, exception_handler, 9);
	IRQ(irq10, exception_handler, 10);
	IRQ(irq11, exception_handler, 11);
	IRQ(irq12, exception_handler, 12);
	IRQ(irq13, exception_handler, 13);
	IRQ(irq14, exception_handler, 14);
	IRQ(irq15, exception_handler, 15);
}

static idtr_t idtr;
static _Bool vectors[IDT_MAX_DESCRIPTORS];


#if 0
__attribute__((noreturn))
void exception_handler_2(void);
void exception_handler_2(void) {
	printk("HELLO\n");
	PIC_acknowledge(1);
	//__asm__ __volatile__("cli; hlt\n");
}
#endif
#if 1
__attribute__((no_caller_saved_registers))
void exception_handler(regs_t *r) {
        const char *exceptions[] = {
        "division by zero",
        "debug",
        "NMI",
        "breakpoint",
        "invalid1",
        "out of bounds",
        "invalid opcode",
        "invalid",
        "double fault",
        "coprocessor segment overrun",
        "bad TSS",
        "segment not present",
        "stack fault",
        "GPF",
        "page fault",
        "unknown interrupt",
        "invalid2",
        "alignment check",
        "machine check",
        "reserved",
        "reserved",
        "reserved",
        "reserved",
        "reserved",
        "reserved",
        "reserved",
        "reserved",
        "reserved",
        "reserved",
        "reserved",
        "reserved",
        "reserved"
    };
		printk("%s\n", __func__);
		printk("r->irq: %d (%x)\n", r->irq, r->irq);
        if (r->irq < 32) {
            panic_irq(exceptions[r->irq]);
        } else {
            (*irq_handlers[r->irq - 32])(r);
        }
        PIC_acknowledge(r->irq);
}
#endif

#define FLAG_SET(number, flag) number |= (flag)
#define FLAG_UNSET(number, flag) number &= ~(flag)


void idt_set_descriptor(uint8_t vector, uintptr_t isr, uint8_t flags,
        uint8_t ist) {
    idt_entry_t *descriptor = &idt[vector];

    descriptor->isr_low        = (uint16_t)(isr & 0xFFFF);
    descriptor->kernel_cs      = GDT_CS64_OFFSET;
	descriptor->bits.ist = 0;
	descriptor->bits.zero = 0;
	descriptor->bits.type = GATE_TYPE_INTERRUPT;
	descriptor->bits.dpl = 0;
	descriptor->bits.p = 1;
    descriptor->isr_mid        = (uint16_t)((isr >> 16) & 0xFFFF);
    descriptor->isr_high       = (uint32_t)((isr >> 32));// & 0xFFFFFFFF);
    descriptor->reserved       = 0;
}

extern uint64_t isr_stub_table[];
static void sanity_check_all_structures(void) {
	printk("idtr: Expected size: 80; Real size: %lu\n", sizeof(idtr) * 8);
	printk("idtr: .base = %x, .limit = %lu\n", idtr.base, idtr.limit);

	printk("idt_entry_t: Expected size: 128; Real size: %lu\n", sizeof(idt[0]) * 8);
}

extern void idt_reload(idtr_t *reg);
void idt_init(void) {
	PIC_enable();
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1;
    for (uint8_t vector = 0; vector < 32; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector],
                IDT_DESCRIPTOR_EXCEPTION, TSS_IST_EXCEPTION);
        vectors[vector] = 1;
    }

	idt_reload(&idtr);

	// shouldn't be needed as there is a function
    //__asm__ __volatile__ ("lidt %0\t\n" : : "m"(idtr)); // load the new IDT
    log_printk("IDT initialized\n");
	sanity_check_all_structures();
	dump_idt_descriptors();
	fill_irq_table();
	//__asm__ __volatile__("hlt");
}

void enable_interrupts(void) {
    __asm__ __volatile__("sti\t\n"); // set the interrupt flag
}
void disable_interrupts(void) {
    __asm__ __volatile__("cli\t\n"); // clear the interrupt flag

}
//#if 0

void __attribute__((no_caller_saved_registers))
kbd_irq_handler(__attribute__((unused)) regs_t *r) {
    unsigned char scan_code = inb(0x60);
	printk("Raw scan code: %x\n", scan_code);
    //__asm__ __volatile__("hlt\t\n");
}
//#endif

