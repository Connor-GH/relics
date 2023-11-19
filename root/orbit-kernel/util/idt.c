#include <idt.h>
#include <pic.h>
#include <kio.h>
#include <keyboard.h>
#include <get_ascii_char.h>
#include <port_functions.h>
#define IDT_MAX_DESCRIPTORS 256
#define GDT_CS64_OFFSET 0x8
#define ISR(isr_stub, n) extern void isr_stub(void); \
    idt_install_handler(isr_stub, n)

#define IRQ(irq, fn, n) extern void irq(void); \
    extern void __attribute__((no_caller_saved_registers)) fn(regs_t *); \
    irq_handlers[n] = fn; \
    idt_install_handler(irq, n + 32)

#define panic_irq(s) printk(s); __asm__ __volatile__("hlt\t\n")
/* https://wiki.osdev.org/Interrupts_Tutorial
 * (based on code licensed under an MIT license) */

/*struct cpu_state {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rbp;
    uint64_t rsi;
    uint64_t rdi;
} __attribute__((packed)); */

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
static interrupt_handler_t idt_handlers[256];
static interrupt_handler_t irq_handlers[256];

void idt_install_handler(void (*handler)(void), int i)
{
    idt_handlers[i] = handler;
}
typedef struct {
    uint16_t        limit;
    uint64_t        base;
} __attribute__((packed)) idtr_t;

void idt_install_handlers()
{
    ISR(isr_stub_0, 0);
    ISR(isr_stub_1, 1);
    ISR(isr_stub_2, 2);
    ISR(isr_stub_3, 3);
    ISR(isr_stub_4, 4);
    ISR(isr_stub_5, 5);
    ISR(isr_stub_6, 6);
    ISR(isr_stub_7, 7);
    ISR(isr_stub_8, 8);
    ISR(isr_stub_9, 9);
    ISR(isr_stub_10, 10);
    ISR(isr_stub_11, 11);
    ISR(isr_stub_12, 12);
    ISR(isr_stub_13, 13);
    ISR(isr_stub_14, 14);
    ISR(isr_stub_15, 15);
    ISR(isr_stub_16, 16);
    ISR(isr_stub_17, 17);
    ISR(isr_stub_18, 18);
    ISR(isr_stub_19, 19);
    ISR(isr_stub_20, 20);
    ISR(isr_stub_21, 21);
    ISR(isr_stub_22, 22);
    ISR(isr_stub_23, 23);
    ISR(isr_stub_24, 24);
    ISR(isr_stub_25, 25);
    ISR(isr_stub_26, 26);
    ISR(isr_stub_27, 27);
    ISR(isr_stub_28, 28);
    ISR(isr_stub_29, 29);
    ISR(isr_stub_30, 30);
    ISR(isr_stub_31, 31);
    /* syscall (TODO: this needs to be in userspace) */
    ISR(isr_stub_128, 128);

    /* Timer */
    //IRQ(irq0, time_source_irq_handler, 0);
    /* Keyboard */
    IRQ(irq1, kbd_irq_handler, 1);
}


static idtr_t idtr;
static _Bool vectors[IDT_MAX_DESCRIPTORS];

/*__attribute__((noreturn))
void exception_handler(void) {
    __asm__ __volatile__
        ("cli\t\n"
            "hlt\t\n"); // Completely hangs the computer
    while (1) {}
}*/
__attribute__((interrupt))
void exception_handler(regs_t *r) {
        const char *exceptions[] = {
        "division by zero",
        "debug",
        "NMI",
        "breakpoint",
        "invalid",
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
        "reserved"
    };
        //PIC_acknowledge(frame->eflags);
        if (r->irq < 32) {
            panic_irq(exceptions[r->irq]);
        } else {
            (*irq_handlers[r->irq - 32])();
        }
        PIC_acknowledge(r->irq);
}

#define FLAG_SET(number, flag) number |= (flag)
#define FLAG_UNSET(number, flag) number &= ~(flag)


void idt_set_descriptor(uint8_t vector, /*void **/uintptr_t isr, uint8_t flags,
        uint8_t ist) {
    idt_entry_t *descriptor = &idt[vector];

    descriptor->isr_low        = isr & 0xFFFF;
    descriptor->kernel_cs      = (uint16_t)GDT_CS64_OFFSET;
    descriptor->ist            = ist;
    descriptor->attributes     = flags;
    descriptor->isr_mid        = (isr >> 16) & 0xFFFF;
    descriptor->isr_high       = (isr >> 32) & 0xFFFFFFFF;
    descriptor->reserved       = 0;
}

//extern /*void **/uint64_t isr_stub_table[];

void idt_init(void) {
    idt_install_handlers();
    PIC_remap(0x20);

#if 0
    for (uint8_t vector = 0; vector < 32; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector],
                IDT_DESCRIPTOR_EXCEPTION, 1/*TSS_IST_EXCEPTION*/);
        vectors[vector] = 1;
    }
#endif
        for (int i = 0; i < 129; i++) {
        uintptr_t base = (uintptr_t) idt_handlers[i];
        idt[i].isr_low  = (base & 0xFFFF);
        idt[i].isr_mid  = (base >> 16) & 0xFFFF;
        idt[i].isr_high = (base >> 32) & 0xFFFFFFFF;
        idt[i].kernel_cs = (uint16_t)GDT_CS64_OFFSET;//0x08;
        idt[i].reserved = 0;
        idt[i].ist = 0;
        idt[i].attributes = 0x8e; /* kernel space only (OR 0) */
    }
    idtr.base = (uintptr_t)&idt/*[0]*/;
    idtr.limit = sizeof(idt);//(uint16_t)sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1;

   // idt_reload(&idtr);

    outb(0x21, 0xfd);
    outb(0x80, 0); // wait for I/O
    outb(0xa1, 0xff);
    __asm__ __volatile__ ("lidt %0\t\n" : : "m"(idtr)); // load the new IDT
    log_printk("IDT initialized\n");
}

void enable_interrupts(void) {
    __asm__ __volatile__("sti\t\n"); // set the interrupt flag
}
void disable_interrupts(void) {
    __asm__ __volatile__("cli\t\n"); // clear the interrupt flag
}
void __attribute__((no_caller_saved_registers))
kbd_irq_handler(__attribute__((unused)) regs_t *r) {
    printk("W\n");
    __asm__ __volatile__("hlt\t\n");
}
