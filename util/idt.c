#include "include/idt.h"

/* TODO: setup IDTs. This file is a WIP */
__attribute__((aligned(0x10)))
static idt_entry_t idt[256]; /* Create an array of IDT entries;
                              * aligned for performance */
static idtr_t idtr;

//__attribute__((noreturn))
void exception_handler() {
    __asm__ __volatile__ ("cli\t\nhlt\t\n"); // Completely hangs the computer
}

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
    idt_entry_t *descriptor = &idt[vector];

    descriptor->isr_low        = (uint32_t)isr & 0xFFFF;
    descriptor->kernel_cs      = 0x08; // this value can be whatever offset your kernel code selector is in your GDT
    descriptor->attributes     = flags;
    descriptor->isr_high       = (uint32_t)isr >> 16;
    descriptor->reserved       = 0;
}

extern void *isr_stub_table[];

void idt_init() {
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(idt_desc_t) * IDT_MAX_DESCRIPTORS - 1;

    for (uint8_t vector = 0; vector < 32; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        vectors[vector] = true;
    }

    __asm__ __volatile__ ("lidt %0\t\n" : : "m"(idtr)); // load the new IDT
    __asm__ __volatile__ ("sti\t\n"); // set the interrupt flag
}
