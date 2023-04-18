#include <idt.h>
#include <port_functions.h>
#define IDT_MAX_DESCRIPTORS 256
#define GDT_CS64_OFFSET 0x8

/* https://wiki.osdev.org/Interrupts_Tutorial
 * (based on code licensed under an MIT license) */

typedef struct {
	uint16_t    isr_low;      // The lower 16 bits of the ISR's address
	uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
	uint8_t	    ist;          // The IST in the TSS that the CPU will load into RSP; set to zero for now
	uint8_t     attributes;   // Type and attributes; see the IDT page
	uint16_t    isr_mid;      // The higher 16 bits of the lower 32 bits of the ISR's address
	uint32_t    isr_high;     // The higher 32 bits of the ISR's address
	uint32_t    reserved;     // Set to zero
} __attribute__((packed)) idt_entry_t;

__attribute__((aligned(16)))
static idt_entry_t idt[IDT_MAX_DESCRIPTORS]; /* Create an array of IDT entries;
                                              * aligned for performance */

typedef struct {
    uint16_t    limit;
    uint64_t    base;
} __attribute__((packed)) idtr_t;

static idtr_t idtr;
static _Bool vectors[IDT_MAX_DESCRIPTORS];

__attribute__((noreturn))
void exception_handler(void) {
    __asm__ __volatile__
        ("cli\t\n"
            "hlt\t\n"); // Completely hangs the computer
    while (1) {}
}

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
    idt_entry_t *descriptor = &idt[vector];

    descriptor->isr_low        = (uint16_t)((uint64_t)isr & 0xFFFF);
    descriptor->kernel_cs      = (uint16_t)GDT_CS64_OFFSET;
    descriptor->ist            = 0;
    descriptor->attributes     = flags;
    descriptor->isr_mid        = ((uint64_t)isr >> 16) & 0xFFFF;
    descriptor->isr_high       = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    descriptor->reserved       = 0;
}

extern void *isr_stub_table[];

void idt_init(void) {
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1;

    for (uint8_t vector = 0; vector < 32; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        vectors[vector] = 1;
    }

    outb(0x21, 0xfd);
    outb(0xa1, 0xff);
    __asm__ __volatile__ ("lidt %0\t\n" : : "m"(idtr)); // load the new IDT
    __asm__ __volatile__ ("sti\t\n"); // set the interrupt flag
}
