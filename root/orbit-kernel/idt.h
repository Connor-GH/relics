#ifndef IDT_H
#define IDT_H
#include "typedefs.h"

struct interrupt_frame {
    uint32_t ip; //64 and rip
    uint32_t cs;
    uint32_t eflags;
    uint32_t sp;
    uint32_t ss;
};
typedef struct __attribute__((packed)) {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
    uint64_t irq;
    uint64_t error_code;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} regs_t;
//void exception_handler(__attribute__((unused)) struct cpu_state cpu,
//        uint32_t interrupt,
//        __attribute__((unused)) struct stack_state stack);

__attribute__((no_caller_saved_registers))
void exception_handler(regs_t *r);

void set_gdt_offset(long offset);

//void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags, uint8_t ist);
void idt_set_descriptor(uint8_t vector, uintptr_t isr, uint8_t flags,
        uint8_t ist);
void idt_init(void);

typedef enum {
    IDT_FLAG_GATE_TASK = 0x5,
    IDT_FLAG_GATE_16BIT_INT = 0x6,
    IDT_FLAG_GATE_16BIT_TRAP = 0x7,
    IDT_FLAG_GATE_32BIT_INT = 0xe,
    IDT_FLAG_GATE_32BIT_TRAP = 0xf,

    IDT_FLAG_RING0 = 0 << 5,
    IDT_FLAG_RING1 = 1 << 5,
    IDT_FLAG_RING2 = 2 << 5,
    IDT_FLAG_RING3 = 3 << 5,

    IDT_FLAG_PRESENT = 0x80,
} IDT_FLAGS;
#define IDT_DESCRIPTOR_EXCEPTION		(IDT_FLAG_GATE_32BIT_INT | IDT_FLAG_PRESENT)
void idt_install_irq_handler(void (*handler)(void), int i);
typedef void (*interrupt_handler_t)(void);
void __attribute__((no_caller_saved_registers)) kbd_irq_handler(regs_t *r);
void enable_interrupts(void);
void disable_interrupts(void);
void irq_install_keyboard(void);

#endif /* IDT_H */
