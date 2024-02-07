#include <idt.h>
#include <pic.h>
#include <kio.h>
#include <ps2_keyboard.h>
#include <orbit.h>
#include <orbit-kernel/string.h>
#define IDT_MAX_DESCRIPTORS 256
#define GDT_CS64_OFFSET ret_gdt_offset()
#define TSS_IST_EXCEPTION 1
#define ISR(isr_stub, n)        \
	extern void isr_stub(void); \
	idt_install_handler(isr_stub, n)

#define PRETTY_PRINT_INT(x) printk("%s: %lu (%x)\n", #x, x, x); do{}while(0)
static void ATTR(noreturn) panic_irq(const char *string) {
	printk(string);
	ASM("cli; hlt\t\n");
	UNREACHABLE();
}

extern uint64_t isr_stub_table[];
extern uint64_t irq_stub_table[];
extern uint16_t ret_gdt_offset(void);

typedef struct {
	uint16_t limit; // limit; must be size of IDT descriptors - 1
	uint64_t base; // base; address of idt descriptors
} __attribute__((packed)) idtr_t;
static idtr_t idtr;

extern void idt_reload(idtr_t *reg);

static volatile uint64_t timer_ticks = 0;

static void (*irq_handlers[IDT_MAX_DESCRIPTORS])(regs_t *) = {0};
static void irq_install_handler(int irq, void (*handler)(regs_t *r)) {
	irq_handlers[irq] = handler;
}

static void decipher_error_code(uint64_t error_code) {

	printk("This error code was caused for the following reasons: \n");
	if (error_code % 2 != 0) {
		printk("- happened due to external hardware (outside of processor)\n");
	}
	printk("- selector index references a descriptor in the ");

	// 0b11 in binary
	switch ((error_code >> 1) & 3) {
		case 0: printk("GDT (0b00)\n"); break;
		case 1: printk("IDT (0b01)\n"); break;
		case 2: printk("LDT (0b10)\n"); break;
		case 3: printk("IDT (0b11)\n"); break;
		default: break;
	}
	printk("In the index: %lu\n", (error_code & 0x0000FFFF) >> 3);
}

typedef struct {
	uint16_t isr_low; // The lower 16 bits of the ISR's address
	uint16_t kernel_cs; // The GDT segment selector that the CPU will load into
						// CS before calling the ISR
	struct idt_bits {
		uint16_t ist : 3, zero : 5, type : 5, dpl : 2, p : 1;
	} __attribute__((packed)) bits;
	uint16_t
		isr_mid; // The higher 16 bits of the lower 32 bits of the ISR's address
	uint32_t isr_high; // The higher 32 bits of the ISR's address
	uint32_t reserved; // Set to zero
} __attribute__((packed)) idt_entry_t; // idt descriptor

static idt_entry_t idt[IDT_MAX_DESCRIPTORS] = {0}; /* Create an array of
													  IDT entries; aligned
													  for performance */

void
exception_handler(regs_t *r)
{
	const char *exceptions[] = { "division by zero",
								 "debug",
								 "NMI",
								 "breakpoint",
								 "invalid",
								 "out of bounds",
								 "invalid opcode",
								 "spurious",
								 "double fault",
								 "coprocessor segment overrun",
								 "bad TSS",
								 "segment not present",
								 "stack fault",
								 "General Protection Fault",
								 "page fault",
								 "unknown interrupt",
								 "invalid",
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
								 "reserved" };
	if (r->irq == 7 + 0x20 || r->irq == 15 + 0x20) {
		PIC_acknowledge(0x20);
		return;
	}
	if (r->irq < 32) {
		printk("**KERNEL FAULT**: ");
		PRETTY_PRINT_INT(r->irq);
		printk("At: %x\n", r->rsp);
		decipher_error_code(r->error_code);
		panic_irq(exceptions[r->irq]);
	} else if (32 <= r->irq && r->irq < 32 + 16) {
		void (*handler)(regs_t *r);
		handler = irq_handlers[r->irq - 32];
		if (handler) {
			handler(r);
		}
		PIC_acknowledge((uint8_t)r->irq);
	}
}


static void
idt_set_descriptor(uint8_t vector, uintptr_t isr, ATTR(unused) uint8_t ist)
{
	idt_entry_t *descriptor = &idt[vector];

	descriptor->isr_low = (uint16_t)(isr & 0xFFFF);
	descriptor->kernel_cs = GDT_CS64_OFFSET;
	descriptor->bits.ist = 0;
	descriptor->bits.zero = 0;
	descriptor->bits.type = IDT_FLAG_GATE_INT;
	descriptor->bits.dpl = 0;
	descriptor->bits.p = 1;
	descriptor->isr_mid = (uint16_t)((isr >> 16) & 0xFFFF);
	descriptor->isr_high = (uint32_t)((isr >> 32)); // & 0xFFFFFFFF);
	descriptor->reserved = 0;

	idtr.base = (uintptr_t)&idt[0];
	idtr.limit = (uint16_t)sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1;
	idt_reload(&idtr);
}


static void
sanity_check_all_structures(void)
{
	if ((sizeof(idtr) * 8) != 80)
		panic_irq("ERROR: idtr: not correct size\n");

	if ((sizeof(idt[0]) * 8) != 128)
		panic_irq("ERROR: idt_entry_t: not correct size\n");
}


void
enable_interrupts(void)
{
	ASM("sti\t\n"); // set the interrupt flag
}
void
disable_interrupts(void)
{
	ASM("cli\t\n"); // clear the interrupt flag
}

static void
kbd_irq_handler(ATTR(unused) regs_t *r)
{
	unsigned char scan_code = inb(0x60);
	printk("%c", to_character(scan_code));
}
static void
timer_handler(ATTR(unused) regs_t *r)
{
	timer_ticks++;
}
static void spurious_interrupt(ATTR(unused) regs_t *r) {
	ASM("nop");
	ASM("nop");
	ASM("nop");
}

static void
fill_irq_table(void)
{
	extern void irq0(regs_t *);
	extern void irq1(regs_t *);
	extern void irq3(regs_t *);
	extern void irq4(regs_t *);
	extern void irq5(regs_t *);
	extern void irq6(regs_t *);
	extern void irq7(regs_t *);
	extern void irq8(regs_t *);
	extern void irq9(regs_t *);
	extern void irq10(regs_t *);
	extern void irq11(regs_t *);
	extern void irq12(regs_t *);
	extern void irq13(regs_t *);
	extern void irq14(regs_t *);
	extern void irq15(regs_t *);
	irq_install_handler(0, timer_handler);
	irq_install_handler(1, kbd_irq_handler);
	irq_install_handler(2, exception_handler);
	irq_install_handler(3, exception_handler);
	irq_install_handler(4, exception_handler);
	irq_install_handler(5, exception_handler);
	irq_install_handler(6, exception_handler);
	irq_install_handler(7, spurious_interrupt);
	irq_install_handler(8, exception_handler);
	irq_install_handler(9, exception_handler);
	irq_install_handler(10, exception_handler);
	irq_install_handler(11, exception_handler);
	irq_install_handler(12, exception_handler);
	irq_install_handler(13, exception_handler);
	irq_install_handler(14, exception_handler);
	irq_install_handler(15, exception_handler);
}
void
idt_init(void)
{
	sanity_check_all_structures();
	memset(idt, 0, sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1);

	for (uint8_t vector = 0; vector < 32; vector++) {
		idt_set_descriptor(vector, isr_stub_table[vector],
				TSS_IST_EXCEPTION);
	}

	for (uint8_t vector = 32; vector < 47; vector++) {
		idt_set_descriptor(vector, irq_stub_table[vector - 32],
				TSS_IST_EXCEPTION);
	}
	fill_irq_table();
	PIC_enable();
	log_printk("IDT initialized\n");
}
